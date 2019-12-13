#include "pch.h"


//start end rear + 1byte.



extern cDump dump;
cRingbuffer::cRingbuffer()
{
	//동적할당 초기화 

	InitializeSRWLock(&ringbuf_srw);

	start = (BYTE*)malloc(erBuffer_defaultsize);

	front = rear = 0;
	peekfront = 0;
	bufsize = erBuffer_defaultsize;

	memset(start, 0, bufsize);

}

cRingbuffer::~cRingbuffer()
{
	free(start);
	start = NULL;
}

void cRingbuffer::Resize(int size)
{
	//동적할당 새로받아서 
	//카피 
	//전에꺼 해제
	//int usesize = Getquesize();

	//BYTE *newstart = (BYTE*)malloc(size + 1);
	//Deque(newstart,usesize); //디큐해서 새로운 버퍼에 넣기

	//free(start);

	//start = newstart;
	//front = 0;
	//rear = usesize;
	//bufsize = size+1;
}

int cRingbuffer::Getquesize()  //현재 사용중인 용량 얻기
{
	//front 에서 rear 까지 용량 확인. 중간에 끊기는지 확인
	if (start == NULL)
		return -1;

	int i_front = front;
	int i_rear = rear;


	if (i_front <= i_rear)
		return i_rear - i_front;
	else
		return (bufsize - i_front) + i_rear;


}

int cRingbuffer::Getquepeeksize()
{
	if (start == NULL)
		return -1;

	int i_front = peekfront;
	int i_rear = rear;


	if (i_front <= i_rear)
		return i_rear - i_front;
	else
		return (bufsize - i_front) + i_rear;
}

int cRingbuffer::Getquefreesize() //현재 사용가능한 (남은)용량 얻기
{
	//rear 에서 front 까지 용량 확인.중간에 끊기는지 확인
	if (start == NULL)
		return -1;

	int i_front = front;
	int i_rear = rear;


	if (i_front <= i_rear)
		return (bufsize - i_rear) + i_front - 1;
	else
		return i_front - i_rear -1;

}

int cRingbuffer::DirectEnquesize() //끊기지않고 한번에 넣을수 있는 용량
{
	//rear 에서 끝까지 또는 front 까지 확인
	if (start == NULL)
		return -1;


	int i_front = front;
	int i_rear = rear;

	if (i_front <= i_rear)
	{
		//if (i_rear == 0)
		//	return bufsize-1;
		if (i_front == 0)
			return bufsize - i_rear - 1;

		return bufsize - i_rear;
	}
	else
		return i_front - i_rear - 1;

}


int cRingbuffer::DirectDequesize()//끊기지않고 한번에 뺄수 있는 용량
{
	//front 에서 rear 까지 또는 끝까지 확인.
	if (start == NULL)
		return -1;

	int i_front = front;
	int i_rear = rear;

	if (i_front <= i_rear)
		return i_rear - i_front;
	else
		return bufsize - i_front;
}
int cRingbuffer::DirectDequepeeksize()
{
	//front 에서 rear 까지 또는 끝까지 확인.
	if (start == NULL)
		return -1;

	int i_front = peekfront;
	int i_rear = rear;

	if (i_front <= i_rear)
		return i_rear - i_front;
	else
		return bufsize - i_front;
}

//인큐 - 데이터 만큼 넣고 rear 위치 옮긴다. 끝났을경우 다시 start 로 가서 쓴다 front 랑 만나면 실패.
int cRingbuffer::Enque(const BYTE* data, int isize)

{
	if (start == NULL)
		return -1;


	int freesize = Getquefreesize();//쓸수있는 용량
	int dirsize = DirectEnquesize();


	if (freesize < isize)
		return -1;



	//남은공간에 데이터 쓸만큼 쓰기  ----------------------------//

	if (dirsize >= isize) //한번에 넣을수 있을때.
	{
		memcpy_s(start+ rear, isize, data, isize); //카피

	}
	else //한번에 못넣을때-> 경계만나는 경우 + 데이터 쓸 공간 작을때.
	{
		memcpy_s(start + rear, dirsize, data, dirsize); //카피
		memcpy_s(start, isize - dirsize, data + dirsize, isize - dirsize);

	}


	rear = (rear + isize) % bufsize;
	return isize;
}

int cRingbuffer::Deque(BYTE* Destbuf, int isize)
{	//디큐 - 데이터 만큼 빼고 front 위치 옮긴다. 위치 확인.
	if (start == NULL)
		return -1;



	//뺄만큼 데이터 없거나 있을때. 
	int usesize = Getquesize();
	int dirsize = DirectDequesize(); //한번에 뺄수 있는 용량 


	if (usesize < isize)
		return 0;


	if (isize <= dirsize)  //한번에 뺄수 있음. 
	{
		memcpy_s(Destbuf, isize, start + front, isize); //카피

	}
	else //한번에 뺄수없음. 경계 걸침.  -  뺄만큼 데이터 없음.
	{
		memcpy_s(Destbuf, dirsize, start + front, dirsize); //카피
		memcpy_s(Destbuf + dirsize, isize - dirsize, start, isize - dirsize);
	}

	front = (front + isize) % bufsize;
	return isize;

}

int cRingbuffer::Peek(BYTE* Destbuf, int isize)
{
	if (start == NULL)
		return -1;


	//디큐에서 front 위치만 안옮기기
	int usesize = Getquesize();
	//뺄만큼 데이터 없거나 있을때. 
	int dirsize = DirectDequesize(); //한번에 뺄수 있는 용량 


	if (usesize < isize)
		return 0;


	if (isize <= dirsize)  //한번에 뺄수 있음. 
	{
		memcpy_s(Destbuf, isize, start + front, isize); //카피
	}
	else //한번에 뺄수없음. 경계 걸침.  -  뺄만큼 데이터 없음.
	{
		memcpy_s(Destbuf, dirsize, start + front, dirsize); //카피
		memcpy_s(Destbuf + dirsize, isize - dirsize, start, isize - dirsize);
	}

	return isize;
}

int cRingbuffer::NextPeek(BYTE* Destbuf, int isize, int pCount)
{
	if (start == NULL)
		return -1;

	//디큐에서 front 위치만 안옮기기
	int usesize = Getquepeeksize();
	//뺄만큼 데이터 없거나 있을때. 
	int dirsize = DirectDequepeeksize(); //한번에 뺄수 있는 용량 

	if (usesize < isize)
		return 0;



	if (isize <= dirsize)  //한번에 뺄수 있음. 
	{
		memcpy_s(Destbuf, isize, start + peekfront, isize); //카피
	}
	else //한번에 뺄수없음. 경계 걸침.  -  뺄만큼 데이터 없음.
	{
		memcpy_s(Destbuf, dirsize, start + peekfront, dirsize); //카피
		memcpy_s(Destbuf + dirsize, isize - dirsize, start, isize - dirsize);
	}

	peekfront = (peekfront + isize) % bufsize;


	return isize;

}

void cRingbuffer::setsendcount(int count)
{
	if (count > 1000)
	{
		dump.Crash();
	}
	sendcount = count;
	return;
}

int cRingbuffer::getsendcount()
{
	return sendcount;
}

int cRingbuffer::Moverear(int isize)
{

	if (start == NULL)
		return -1;

	int freesize = Getquefreesize();//쓸수있는 용량


	if (freesize < isize)
		return -1;


	rear = (rear + isize) % bufsize;
	return isize;

}


int cRingbuffer::Movefront(int isize)
{	
	if (start == NULL)
		return -1;

	int i_front = front;
	int i_rear = rear;

	//뺄만큼 데이터 없거나 있을때. 
	int usesize = Getquesize();



	if (usesize < isize)
		return 0;

	front = (front + isize) % bufsize;
	return isize;


}

void cRingbuffer::Clearbuf(void)//모든 데이터 삭제
{
	//front 와 rear 초기화 

	front =0;
	rear = 0;
	peekfront = 0;

}

BYTE* cRingbuffer::Getbufptr(void)
{
	return start;
}

BYTE* cRingbuffer::Getfrontptr(void)
{
	//front 위치 리턴
	return start + front;
}

BYTE* cRingbuffer::Getrearptr(void)
{

	return start + rear;
}

void cRingbuffer::Lock()
{
	AcquireSRWLockExclusive(&ringbuf_srw);
}


void cRingbuffer::Unlock()
{
	ReleaseSRWLockExclusive(&ringbuf_srw);
}
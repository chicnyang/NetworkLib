#include "pch.h"


//start end rear + 1byte.

long cDump::dumpcount;
extern cDump dump;

cRingbuffer::cRingbuffer()
{
	//�����Ҵ� �ʱ�ȭ 

	InitializeSRWLock(&ringbuf_srw);

	start = (BYTE*)malloc(erBuffer_defaultsize);

	front = rear = 0;
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
	//�����Ҵ� ���ι޾Ƽ� 
	//ī�� 
	//������ ����
	//int usesize = Getquesize();

	//BYTE *newstart = (BYTE*)malloc(size + 1);
	//Deque(newstart,usesize); //��ť�ؼ� ���ο� ���ۿ� �ֱ�

	//free(start);

	//start = newstart;
	//front = 0;
	//rear = usesize;
	//bufsize = size+1;
}

int cRingbuffer::Getquesize()  //���� ������� �뷮 ���
{
	//front ���� rear ���� �뷮 Ȯ��. �߰��� ������� Ȯ��
	if (start == NULL)
		return -1;

	int i_front = front;
	int i_rear = rear;


	if (i_front <= i_rear)
		return i_rear - i_front;
	else
		return (bufsize - i_front) + i_rear;


}

int cRingbuffer::Getquefreesize() //���� ��밡���� (����)�뷮 ���
{
	//rear ���� front ���� �뷮 Ȯ��.�߰��� ������� Ȯ��
	if (start == NULL)
		return -1;

	int i_front = front;
	int i_rear = rear;


	if (i_front <= i_rear)
		return (bufsize - i_rear) + i_front - 1;
	else
		return i_front - i_rear -1;

}

int cRingbuffer::DirectEnquesize() //�������ʰ� �ѹ��� ������ �ִ� �뷮
{
	//rear ���� ������ �Ǵ� front ���� Ȯ��
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


int cRingbuffer::DirectDequesize()//�������ʰ� �ѹ��� ���� �ִ� �뷮
{
	//front ���� rear ���� �Ǵ� ������ Ȯ��.
	if (start == NULL)
		return -1;

	int i_front = front;
	int i_rear = rear;

	if (front <= i_rear)
		return i_rear - i_front;
	else
		return bufsize - i_front;
}


//��ť - ������ ��ŭ �ְ� rear ��ġ �ű��. ��������� �ٽ� start �� ���� ���� front �� ������ ����.
int cRingbuffer::Enque(const BYTE* data, int isize)

{
	if (start == NULL)
		return -1;


	int freesize = Getquefreesize();//�����ִ� �뷮
	int dirsize = DirectEnquesize();


	if (freesize < isize)
		return 0;



	//���������� ������ ����ŭ ����  ----------------------------//

	if (dirsize >= isize) //�ѹ��� ������ ������.
	{
		memcpy_s(start+ rear, isize, data, isize); //ī��

		//if (rear + isize == bufsize)
		//	rear = 0;
		//else
		//	rear += isize;

		 
		//return isize;
	}
	else //�ѹ��� ��������-> ��踸���� ��� + ������ �� ���� ������.
	{
		memcpy_s(start + rear, dirsize, data, dirsize); //ī��
		memcpy_s(start, isize - dirsize, data + dirsize, isize - dirsize);

		//rear = isize - dirsize;
		//return isize;
	}


	rear = (rear + isize) % bufsize;
	return isize;
}

int cRingbuffer::Deque(BYTE* Destbuf, int isize)
{	//��ť - ������ ��ŭ ���� front ��ġ �ű��. ��ġ Ȯ��.
	if (start == NULL)
		return -1;



	//����ŭ ������ ���ų� ������. 
	int usesize = Getquesize();
	int dirsize = DirectDequesize(); //�ѹ��� ���� �ִ� �뷮 


	if (usesize < isize)
		return 0;


	if (isize <= dirsize)  //�ѹ��� ���� ����. 
	{
		memcpy_s(Destbuf, isize, start + front, isize); //ī��

	}
	else //�ѹ��� ��������. ��� ��ħ.  -  ����ŭ ������ ����.
	{
		memcpy_s(Destbuf, dirsize, start + front, dirsize); //ī��
		memcpy_s(Destbuf + dirsize, isize - dirsize, start, isize - dirsize);
	}

	front = (front + isize) % bufsize;
	return isize;

}

int cRingbuffer::Peek(BYTE* Destbuf, int isize)
{
	if (start == NULL)
		return -1;


	//��ť���� front ��ġ�� �ȿű��
	int usesize = Getquesize();
	//����ŭ ������ ���ų� ������. 
	int dirsize = DirectDequesize(); //�ѹ��� ���� �ִ� �뷮 


	if (usesize < isize)
		return 0;


	if (isize <= dirsize)  //�ѹ��� ���� ����. 
	{
		memcpy_s(Destbuf, isize, start + front, isize); //ī��
	}
	else //�ѹ��� ��������. ��� ��ħ.  -  ����ŭ ������ ����.
	{
		memcpy_s(Destbuf, dirsize, start + front, dirsize); //ī��
		memcpy_s(Destbuf + dirsize, isize - dirsize, start, isize - dirsize);
	}

	return isize;
}

int cRingbuffer::Moverear(int isize)
{

	if (start == NULL)
		return -1;

	int freesize = Getquefreesize();//�����ִ� �뷮


	if (freesize < isize)
		return 0;


	rear = (rear + isize) % bufsize;
	return isize;

}


int cRingbuffer::Movefront(int isize)
{
	if (start == NULL)
	{
		dump.Crash();
		return -1;
	}


	int i_front = front;
	int i_rear = rear;

	//����ŭ ������ ���ų� ������. 
	int usesize = Getquesize();



	if (usesize < isize)
	{
		dump.Crash();
		LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"front  %d , rear  %d , usesize  %d ", front, rear, usesize);
		return 0;
	}


	front = (front + isize) % bufsize;
	return isize;


}

void cRingbuffer::Clearbuf(void)//��� ������ ����
{
	//front �� rear �ʱ�ȭ 

	front =0;
	rear = 0;

}

BYTE* cRingbuffer::Getbufptr(void)
{
	return start;
}

BYTE* cRingbuffer::Getfrontptr(void)
{
	//front ��ġ ����
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
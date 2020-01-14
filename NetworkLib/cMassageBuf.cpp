#include "pch.h"

cMemorypoolTLS<cMassage>* cMassage::packetPool;

// LONG cMassage::debugcount;
 //char cMassage::debugbuf[10000];


cMassage::cMassage()
{
	//hHeap = HeapCreate(0,0,0);
	//pBuf = (BYTE*)HeapAlloc(hHeap,0, eBuffer_defaultsize);
	msgptr = (BYTE*)malloc(eBuffer_defaultsize);

	pHeader = msgptr;
	pBuf = pHeader + 5;

	front = 0;
	rear = 0;
	refcount = 0;
	bufsize = eBuffer_defaultsize;
}
//cMassage::cMassage(int ibufsize)
//{
	//hHeap = HeapCreate(0, 0, 0);
	//pBuf = (BYTE*)HeapAlloc(hHeap, 0, ibufsize);

	//bufUsesize = 0;
	//bufsize = ibufsize;
	//front = 0;
	//rear = 0;
//}

void cMassage::resize(int size)
{
	//BYTE* pNewBuf;
	//int newsize;

	//int logsize = size - (bufsize - bufUsesize); //모자란 공간.

	//if (size + bufsize > bufsize * 2)
	//{
	//	newsize = size + bufsize;
	//}
	//else
	//	newsize = bufsize * 2;

	//if (bSizenotuse)
	//{
	//	pNewBuf = (BYTE*)HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pBuf, newsize);  //0으로 채워짐. 
	//	pBuf = pNewBuf;
	//	bufsize = newsize;
	//	return;
	//}

	//pNewBuf = (BYTE*)HeapReAlloc(hHeap,HEAP_ZERO_MEMORY, pBuf, newsize);
	//pBuf = pNewBuf;
	//bufsize = newsize;

	//logfile_create(serialization_buf_resize, logsize);

}

void cMassage::settingHeader(int byte)
{
	headersize = byte;
	WORD size = bufUsesize;

	if (bufUsesize == 0)
	{
		dump.Crash();
	}


	switch (headersize)
	{
	case 2:
		memcpy_s(pHeader + (5 - headersize), sizeof(WORD), &size, sizeof(WORD));
		break;
	case 5:
		//나중에 세팅할것.
		break;

	default:
		break;
	}
	
}


//메시지 파괴  -  버퍼 지우기 
void cMassage::Release(void)
{
//	HeapDestroy(hHeap);
	free(msgptr);
}

void cMassage::MemoryPool(int size)
{
	packetPool = new cMemorypoolTLS<cMassage>;
}

cMassage* cMassage::Alloc()
{



	cMassage* msg = packetPool->alloc();
	int rciunt = msg->refcount;
	if(rciunt != 0)
	{
		dump.Crash();
	}
	msg->bAlloc = true;

	msg->refcntUp();
	msg->Clear();
	return msg;
}

void cMassage::Free()
{
	//InterlockedIncrement(&packetcount);
	//if (packetcount == 10000)
	//{
	//	packetcount = 0;
	//}
	//packetdebug[packetcount] = 'f';

	int ret = InterlockedDecrement(&refcount);
	if (ret == 0)
	{
		this->bAlloc = false;
		//this->Clear();
		packetPool->free(this);
	}
	else if(ret<0)
	{
		dump.Crash();
	}
}

//메시지 청소  -  버퍼 비우기 
void cMassage::Clear(void)
{

	//InterlockedIncrement(&packetcount);
	//if (packetcount == 10000)
	//{
	//	packetcount = 0;
	//}
	//packetdebug[packetcount] = 'c';

	sendflag = false;

	front = 0;
	rear = 0;
	bufUsesize = 0;
	headersize = 0;
}

//버퍼 사이즈 얻기
int cMassage::Getbufsize(void)
{
	return bufsize;
}

//사용중인 사이즈 얻기
int cMassage::Getusesize(void)
{
	return bufUsesize;
}

BYTE* cMassage::GetHeaderbufferptr(void)
{
	return pHeader + (5 - headersize);
}

//버퍼포인터 얻기
BYTE* cMassage::Getbufferptr(void)
{
	return pBuf + front;
}

//버퍼 ptr offset 이동.
int cMassage::MoveWritepos(int pos)
{
	rear += pos;
	if (bufsize < rear)
	{
		return -1;
	}
	bufUsesize = rear - front;

	packetsize = bufUsesize;

	return pos;
}


int cMassage::MoveReadpos(int pos)
{
	front += pos;

	if (rear < front)
		return -1;

	bufUsesize = rear - front;
	return pos;
}


//연산자 오버로딩 넣기 빼기
cMassage &cMassage::operator = (cMassage &srcMassage)
{
	//++packetcount;
	//if (packetcount == 1000)
	//{
	//	packetcount = 0;
	//}
	//packetdebug[packetcount] = '=';

	cMassage *pMsg = &srcMassage;

	if(bufsize - bufUsesize < pMsg->Getusesize())
	{
		resize(pMsg->Getusesize());
	}

	memcpy_s(pBuf+rear, bufsize - bufUsesize,pMsg->Getbufferptr(),pMsg->Getusesize());
	rear += pMsg->Getusesize();
	bufUsesize = rear - front;

	packetsize = bufUsesize;

	return *this;
}

//넣기 
cMassage &cMassage::operator << (BYTE bValue)
{
	if (bufsize - bufUsesize < sizeof(bValue))
	{
		resize(sizeof(bValue));
	}


	memcpy_s(pBuf + rear, bufsize - bufUsesize, &bValue,sizeof(bValue));
	rear += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}
cMassage &cMassage::operator << (char bValue)
{

	if (bufsize - bufUsesize < sizeof(bValue))
	{
		resize(sizeof(bValue));
	}

	memcpy_s(pBuf + rear, bufsize - bufUsesize, &bValue, sizeof(bValue));
	rear += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator << (short bValue)
{
	if (bufsize - bufUsesize < sizeof(bValue))
	{
		resize(sizeof(bValue));
	}

	memcpy_s(pBuf + rear, bufsize - bufUsesize, &bValue, sizeof(bValue));
	rear += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator << (WORD bValue)
{


	if (bufsize - bufUsesize < sizeof(bValue))
	{
		resize(sizeof(bValue));
	}

	memcpy_s(pBuf + rear, bufsize - bufUsesize, &bValue, sizeof(bValue));
	rear += sizeof(bValue);

	bufUsesize = rear - front;


	return *this;
}

cMassage &cMassage::operator << (int bValue)
{
	if (bufsize - bufUsesize < sizeof(bValue))
	{
		resize(sizeof(bValue));
	}


	memcpy_s(pBuf + rear, bufsize - bufUsesize, &bValue, sizeof(bValue));
	rear += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}
cMassage &cMassage::operator << (float bValue)
{
	if (bufsize - bufUsesize < sizeof(bValue))
	{
		resize(sizeof(bValue));
	}


	memcpy_s(pBuf + rear, bufsize - bufUsesize, &bValue, sizeof(bValue));
	rear += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator << (DWORD bValue)
{
	if (bufsize - bufUsesize < sizeof(bValue))
	{
		resize(sizeof(bValue));
	}

	memcpy_s(pBuf + rear, bufsize - bufUsesize, &bValue, sizeof(bValue));
	rear += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;

}
cMassage &cMassage::operator << (double bValue)
{
	if (bufsize - bufUsesize < sizeof(bValue))
	{
		resize(sizeof(bValue));
	}

	memcpy_s(pBuf + rear, bufsize - bufUsesize, &bValue, sizeof(bValue));
	rear += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}
cMassage &cMassage::operator << (__int64 bValue)
{
	if (bufsize - bufUsesize < sizeof(bValue))
	{
		resize(sizeof(bValue));
	}


	memcpy_s(pBuf + rear, bufsize - bufUsesize, &bValue, sizeof(bValue));
	rear += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}


//빼기
//패킷하나만을 담고 버림. 그러므로 패킷중 어느부분이 문제인지 굳이 확인할 필요는 없음.
//큰값을 빼라고 들어왓을때 리사이즈로 다른값을 채우고 빼주고 변수로 하나 에러 메시지를 둬서 패킷을 로그에 찍어서 확인 패킷은 폐기한다. Getlasterror 방식.
//이유- 밖에서 패킷의 사이즈를 확인하지않고 사이즈만큼만 긁어오는데 그럼 패킷의 사이즈를 굳이 모른다는 뜻. 그럼 안에들어있는 데이터가 실제 패킷의 데이터와 일치하는지 확인하기는 좀 그럴듯.
//만약 밖에서확인할거 였다면 헤더 읽을때 이미 타입과 사이즈가 일치하는지 확인했어야 함.
//try catch 로 바꿀것. 

cMassage &cMassage::operator >> (BYTE &bValue)
{
	if (bufUsesize < sizeof(bValue))
	{
		WCHAR buf[10] = { 0 };
		memcpy_s(buf, sizeof(buf), pBuf + front, bufUsesize);

		//throw Exception((WCHAR*)__FILE__,__LINE__, buf);
	}


	memcpy_s(&bValue, sizeof(bValue) , pBuf + front, sizeof(bValue));
	front += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator >> (char &bValue)
{


	if (bufUsesize < sizeof(bValue))
	{
		WCHAR buf[10] = { 0 };
		memcpy_s(buf, sizeof(buf), pBuf + front, bufUsesize);

		//throw Exception((WCHAR*)__FILE__, __LINE__, buf);
	}

	memcpy_s(&bValue, sizeof(bValue), pBuf + front, sizeof(bValue));
	front += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator >> (short &bValue)
{
	if (bufUsesize < sizeof(bValue))
	{
		WCHAR buf[10] = { 0 };
		memcpy_s(buf, sizeof(buf), pBuf + front, bufUsesize);

		//throw Exception((WCHAR*)__FILE__, __LINE__, buf);
	}

	memcpy_s(&bValue, sizeof(bValue), pBuf + front, sizeof(bValue));
	front += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator >> (WORD &bValue)
{
	if (bufUsesize < sizeof(bValue))
	{
		WCHAR buf[10] = { 0 };
		memcpy_s(buf, sizeof(buf), pBuf + front, bufUsesize);

		//throw Exception((WCHAR*)__FILE__, __LINE__, buf);
	}

	memcpy_s(&bValue, sizeof(bValue), pBuf + front, sizeof(bValue));
	front += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator >> (int &bValue)
{
	if (bufUsesize < sizeof(bValue))
	{
		WCHAR buf[10] = { 0 };
		memcpy_s(buf, sizeof(buf), pBuf + front, bufUsesize);

		//throw Exception((WCHAR*)__FILE__, __LINE__, buf);
	}

	memcpy_s(&bValue, sizeof(bValue), pBuf + front, sizeof(bValue));
	front += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator >> (float &bValue)
{
	if (bufUsesize < sizeof(bValue))
	{
		WCHAR buf[10] = { 0 };
		memcpy_s(buf, sizeof(buf), pBuf + front, bufUsesize);

		//throw Exception((WCHAR*)__FILE__, __LINE__, buf);
	}

	memcpy_s(&bValue, sizeof(bValue), pBuf + front, sizeof(bValue));
	front += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator >> (DWORD &bValue)
{
	if (bufUsesize < sizeof(bValue))
	{
		WCHAR buf[10] = { 0 };
		memcpy_s(buf, sizeof(buf), pBuf + front, bufUsesize);

		//throw Exception((WCHAR*)__FILE__, __LINE__, buf);
	}

	memcpy_s(&bValue, sizeof(bValue), pBuf + front, sizeof(bValue));
	front += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator >> (double &bValue)
{
	if (bufUsesize < sizeof(bValue))
	{
		WCHAR buf[10] = { 0 };
		memcpy_s(buf, sizeof(buf), pBuf + front, bufUsesize);

		//throw Exception((WCHAR*)__FILE__, __LINE__, buf);
	}

	memcpy_s(&bValue, sizeof(bValue), pBuf + front, sizeof(bValue));
	front += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}

cMassage &cMassage::operator >> (__int64 &bValue)
{

	if (bufUsesize < sizeof(bValue))
	{
		WCHAR buf[10] = { 0 };
		memcpy_s(buf, sizeof(buf), pBuf + front, bufUsesize);

		//throw Exception((WCHAR*)__FILE__, __LINE__, buf);
	}

	memcpy_s(&bValue, sizeof(bValue), pBuf + front, sizeof(bValue));
	front += sizeof(bValue);
	bufUsesize = rear - front;
	return *this;
}


//데이터 얻기 
int cMassage::GetData(BYTE *Dest, int iSize)
{
	if (iSize < 0)
		return -1;


	if (bufUsesize < iSize)
	{
		bSizenotuse = true;
		resize(iSize);
	}

	memcpy_s(Dest, iSize, pBuf + front, iSize);
	front += iSize;

	bufUsesize = rear-front;
	return iSize;
}

//데이터 삽입
int cMassage::InputData(BYTE *Src, int iSize)
{



	if (iSize < 0)
		return -1;

	if (bufsize - bufUsesize < iSize)
	{
		resize(iSize);
	}

	memcpy_s(pBuf + rear, iSize, Src, iSize);
	rear += iSize;


	if (rear > bufsize)
		return -1;

	bufUsesize = rear - front;


	return iSize;
}

//빼다가 오류 났는지 확인
BOOL cMassage::sizeerr()
{
	return bSizenotuse;
}

int cMassage::GetUsePacket()
{
	return 	packetPool->usecount();
}

int cMassage::GetsizePacketPool()
{
	return packetPool->alloccount();
}

void cMassage::refcntUp()
{
	//InterlockedIncrement(&packetcount);
	//if (packetcount == 10000)
	//{
	//	packetcount = 0;
	//}
	//packetdebug[packetcount] = 'u';

	if (InterlockedIncrement(&refcount) == 0)
		dump.Crash();
}

void cMassage::refcntDown()
{
	//InterlockedIncrement(&packetcount);
	//if (packetcount == 10000)
	//{
	//	packetcount = 0;
	//}
	//packetdebug[packetcount] = 'd';
	if(InterlockedDecrement(&refcount) < 0)
		dump.Crash();
}

int cMassage::GetHeaderusesize(void)
{
	return headersize + bufUsesize;
}
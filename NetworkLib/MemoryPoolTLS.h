#include <windows.h>
#include "MemoryPool_stack.h"

#ifndef __MEMORYPOOL_TLS__
#define __MEMORYPOOL_TLS__

#define CHKSUM 0x79ffff79
#define ARRAYMAX 100

// Chunk 클래스 
extern cDump dump;
template<class Data>
class cChunk
{
public:


	cChunk()
	{
		//chunk 요소 초기화 
		index = 0;
		freecount = 0;

		//배열생성 돌면서 포인터 넣기
		for (int i = 0; i < ARRAYMAX; i++)
		{
			new(&Nodearray[i].data) Data();
			Nodearray[i].mychunk = this;
			Nodearray[i].chksum = CHKSUM;
		}
		
		

	}
	virtual ~cChunk() {}

	Data* alloc(void)
	{
		Data* retdata = nullptr;
		//인덱스 ++ 하면서 리턴

		if (index < ARRAYMAX)
		{
			retdata = &(Nodearray[index].data);
			index++;
		}

		return retdata;
	}
	BOOL free(Data* pData)
	{
		Node* node = (Node*)pData;

		if(node->chksum != CHKSUM)
		{
			dump.Crash();
		}

		if (100 == InterlockedIncrement(&freecount))
		{
			return false;
		}
		return true;
	}

	//배열로 데이터 관리 
	struct Node
	{

		Data data;
		int chksum;
		cChunk<Data>* mychunk;
		~Node() {}
	};

	void clear()
	{
		index = 0;
		freecount = 0;
	}
	int index;

	char debug[10000] = { 0, };
	LONG dcount = 0;
private:

	//노드 배열
	Node Nodearray[ARRAYMAX];
	//index + freecount

	LONG freecount;

};



//메모리풀 전체 총괄 클래스 
template<class Data>
class cMemorypoolTLS
{
public:
	cMemorypoolTLS()
	{
		//chunk 관리 메모리풀 생성
		memorypool = new cMemoryPool<cChunk<Data>>(0);
		//TLS 인덱스 받기
		TLSindex = TlsAlloc();
	}
	~cMemorypoolTLS() {}


	Data* alloc(void)
	{
		//내 스레드가 받은 청크 있는지 확인			
		cChunk<Data>* mychunk = (cChunk<Data>*)TlsGetValue(TLSindex);
		int a = 0;
		//받은 청크 없다면 다시 받기 
		if (mychunk == nullptr)
		{
			a = 1;
			mychunk = memorypool->alloc();

			int count = InterlockedIncrement(&mychunk->dcount);
			if (count > 10000)
			{
				InterlockedExchange(&mychunk->dcount, 0);
			}
			mychunk->debug[count] = 'a';
			TlsSetValue(TLSindex,mychunk);
		}

		//청크 있다면 청크에서 받기
		Data* retdata = mychunk->alloc();
		if(retdata == nullptr)
		{
			a = 2;
			//청크에서 더이상 데이터 못뺄때 청크 다시받기 

			mychunk = memorypool->alloc();
			mychunk->clear();
			int count = InterlockedIncrement(&mychunk->dcount);
			if (count > 10000)
			{
				InterlockedExchange(&mychunk->dcount, 0);
			}
			mychunk->debug[count] = 'a';
			TlsSetValue(TLSindex, mychunk);
			retdata = mychunk->alloc();
		}
		return retdata;
	}
	BOOL free(Data* pData)
	{
		//청크 free 호출
		//false 시 100 개 채운거 + 반환해야함.
		//cChunk<Data>::Node Mynode;

		typename cChunk<Data>::Node* node = (typename cChunk<Data>::Node*)pData;

		cChunk<Data>* mychunk = node->mychunk;

		if (!mychunk->free(pData))
		{
			//실제로 반환
			mychunk->clear();
			memorypool->free(mychunk);
			int count=InterlockedIncrement(&mychunk->dcount);
			if (count > 10000)
			{
				InterlockedExchange(&mychunk->dcount,0);
			}
			mychunk->debug[count] = 'f';
		}

		return true;
	}
	//alloc 갯수 - 할당 받아둔 갯수 

	int alloccount(void)
	{
		return memorypool->alloccount() * ARRAYMAX;
	}


	//use 갯수  -  사용자가 지금 쓰고있는 갯수
	int usecount(void)
	{
		return memorypool->usecount() * ARRAYMAX;
	}


private:


	cMemoryPool<cChunk<Data>>* memorypool;
	DWORD TLSindex;
};


#endif // !__MEMORYPOOL_TLS_



#include <windows.h>
#include "MemoryPool_stack.h"

#ifndef __MEMORYPOOL_TLS__
#define __MEMORYPOOL_TLS__

#define CHKSUM 0x79ffff79
#define ARRAYMAX 100

// Chunk 클래스 
extern cDump dump;


//메모리풀 전체 총괄 클래스 
template<class Data>
class cMemorypoolTLS
{
private:

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

			if (node->chksum != CHKSUM)
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
			cMemorypoolTLS<Data>::cChunk* mychunk;
		};
		void clear()
		{
			index = 0;
			freecount = 0;
		}

		//char debug[10000] = { 0, };
		//LONG dcount = 0;


		//노드 배열
		Node Nodearray[ARRAYMAX];
		//index + freecount
		LONG index;
		LONG freecount;

	};
public:
	cMemorypoolTLS()
	{
		//chunk 관리 메모리풀 생성
		memorypool = new cMemoryPool<cChunk>(0);
		//TLS 인덱스 받기
		TLSindex = TlsAlloc();
	}
	~cMemorypoolTLS() {}


	Data* alloc(void)
	{
		//내 스레드가 받은 청크 있는지 확인			
		cChunk* mychunk = (cChunk*)TlsGetValue(TLSindex);
		int a = 0;
		//받은 청크 없다면 다시 받기 
		if (mychunk == nullptr)
		{
			a = 1;
			mychunk = memorypool->alloc();
			mychunk->clear();
			//int count = InterlockedIncrement(&mychunk->dcount);
			//if (count > 10000)
			//{
			//	InterlockedExchange(&mychunk->dcount, 0);
			//}
			//mychunk->debug[count] = 'a';
			TlsSetValue(TLSindex,mychunk);
		}

		//청크 있다면 청크에서 받기
		Data* retdata = mychunk->alloc();

		//청크에서 더이상 데이터 못뺄때 청크 다시받아 두기 
		if(mychunk->index ==100)
		{
			mychunk = memorypool->alloc();
			//alloc 후 clear
			mychunk->clear();
			//int count = InterlockedIncrement(&mychunk->dcount);
			//if (count > 10000)
			//{
			//	InterlockedExchange(&mychunk->dcount, 0);
			//}
			//mychunk->debug[count] = 'a';
			TlsSetValue(TLSindex, mychunk);
		}

		InterlockedIncrement(&use_count);
		return retdata;
	}
	BOOL free(Data* pData)
	{
		//청크 free 호출
		//false 시 100 개 채운거 + 반환해야함.
		//cChunk<Data>::Node Mynode;

		typename cChunk::Node* node = (typename cChunk::Node*)pData;

		cChunk* mychunk = node->mychunk;

		if (!mychunk->free(pData))
		{
			//실제로 반환
			memorypool->free(mychunk);
			//int count=InterlockedIncrement(&mychunk->dcount);
			//if (count > 10000)
			//{
			//	InterlockedExchange(&mychunk->dcount,0);
			//}
			//mychunk->debug[count] = 'f';
		}
		InterlockedDecrement(&use_count);
		return true;
	}
	//alloc 갯수 - 할당 받아둔 갯수 

	int alloccount(void)
	{
		return memorypool->alloccount()*ARRAYMAX;
	}
	//use 갯수  -  사용자가 지금 쓰고있는 갯수
	int usecount(void)
	{
		return use_count;
	}


private:
	cMemoryPool<cChunk>* memorypool;
	DWORD TLSindex;
	LONG use_count;
};


#endif // !__MEMORYPOOL_TLS_



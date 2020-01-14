#include <windows.h>
#include "MemoryPool_stack.h"

#ifndef __MEMORYPOOL_TLS__
#define __MEMORYPOOL_TLS__

#define CHKSUM 0x79ffff79
#define ARRAYMAX 100

// Chunk Ŭ���� 
extern cDump dump;


//�޸�Ǯ ��ü �Ѱ� Ŭ���� 
template<class Data>
class cMemorypoolTLS
{
private:

	class cChunk
	{
	public:
		cChunk()
		{
			//chunk ��� �ʱ�ȭ 
			index = 0;
			freecount = 0;

			//�迭���� ���鼭 ������ �ֱ�
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
			//�ε��� ++ �ϸ鼭 ����

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
		//�迭�� ������ ���� 
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


		//��� �迭
		Node Nodearray[ARRAYMAX];
		//index + freecount
		LONG index;
		LONG freecount;

	};
public:
	cMemorypoolTLS()
	{
		//chunk ���� �޸�Ǯ ����
		memorypool = new cMemoryPool<cChunk>(0);
		//TLS �ε��� �ޱ�
		TLSindex = TlsAlloc();
	}
	~cMemorypoolTLS() {}


	Data* alloc(void)
	{
		//�� �����尡 ���� ûũ �ִ��� Ȯ��			
		cChunk* mychunk = (cChunk*)TlsGetValue(TLSindex);
		int a = 0;
		//���� ûũ ���ٸ� �ٽ� �ޱ� 
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

		//ûũ �ִٸ� ûũ���� �ޱ�
		Data* retdata = mychunk->alloc();

		//ûũ���� ���̻� ������ ������ ûũ �ٽù޾� �α� 
		if(mychunk->index ==100)
		{
			mychunk = memorypool->alloc();
			//alloc �� clear
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
		//ûũ free ȣ��
		//false �� 100 �� ä��� + ��ȯ�ؾ���.
		//cChunk<Data>::Node Mynode;

		typename cChunk::Node* node = (typename cChunk::Node*)pData;

		cChunk* mychunk = node->mychunk;

		if (!mychunk->free(pData))
		{
			//������ ��ȯ
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
	//alloc ���� - �Ҵ� �޾Ƶ� ���� 

	int alloccount(void)
	{
		return memorypool->alloccount()*ARRAYMAX;
	}
	//use ����  -  ����ڰ� ���� �����ִ� ����
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



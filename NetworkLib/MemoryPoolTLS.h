#include <windows.h>
#include "MemoryPool_stack.h"

#ifndef __MEMORYPOOL_TLS__
#define __MEMORYPOOL_TLS__

#define CHKSUM 0x79ffff79
#define ARRAYMAX 100

// Chunk Ŭ���� 
extern cDump dump;
template<class Data>
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

	//�迭�� ������ ���� 
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

	//��� �迭
	Node Nodearray[ARRAYMAX];
	//index + freecount

	LONG freecount;

};



//�޸�Ǯ ��ü �Ѱ� Ŭ���� 
template<class Data>
class cMemorypoolTLS
{
public:
	cMemorypoolTLS()
	{
		//chunk ���� �޸�Ǯ ����
		memorypool = new cMemoryPool<cChunk<Data>>(0);
		//TLS �ε��� �ޱ�
		TLSindex = TlsAlloc();
	}
	~cMemorypoolTLS() {}


	Data* alloc(void)
	{
		//�� �����尡 ���� ûũ �ִ��� Ȯ��			
		cChunk<Data>* mychunk = (cChunk<Data>*)TlsGetValue(TLSindex);
		int a = 0;
		//���� ûũ ���ٸ� �ٽ� �ޱ� 
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

		//ûũ �ִٸ� ûũ���� �ޱ�
		Data* retdata = mychunk->alloc();
		if(retdata == nullptr)
		{
			a = 2;
			//ûũ���� ���̻� ������ ������ ûũ �ٽùޱ� 

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
		//ûũ free ȣ��
		//false �� 100 �� ä��� + ��ȯ�ؾ���.
		//cChunk<Data>::Node Mynode;

		typename cChunk<Data>::Node* node = (typename cChunk<Data>::Node*)pData;

		cChunk<Data>* mychunk = node->mychunk;

		if (!mychunk->free(pData))
		{
			//������ ��ȯ
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
	//alloc ���� - �Ҵ� �޾Ƶ� ���� 

	int alloccount(void)
	{
		return memorypool->alloccount() * ARRAYMAX;
	}


	//use ����  -  ����ڰ� ���� �����ִ� ����
	int usecount(void)
	{
		return memorypool->usecount() * ARRAYMAX;
	}


private:


	cMemoryPool<cChunk<Data>>* memorypool;
	DWORD TLSindex;
};


#endif // !__MEMORYPOOL_TLS_



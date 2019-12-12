#pragma once
#include <new>
#include <Windows.h>


//namespace ���������� ���� �ߺ��̸� ������ �������.


/*
template <class Data>
class objectListPool
{
	//���� ����� ������
	struct Node
	{
		Node()
		{
			Nextnode = NULL;
		}
		Node* Nextnode;
	};

	//������ �ı��� 
	//false �� ������ �ı��� ȣ���� �ȿ��� ����. 
	objectListPool(int poolsize, bool bcreatecall = false)
	{
		if (poolsize == 0) //����Ʈ�ǰ�� 
		{

		}
		else
		{
		}

	}

	virtual ~objectListPool(void)
	{

	}

	 //alloc
	 Data* alloc(void)
	 {

	 }
	 // p = malloc(sizeof(Node) + sizeof(Data));

	 //free
	 BOOL free(Data* pData)
	 {

	 }


	 //alloc ���� - �Ҵ� �޾Ƶ� ���� 

	 int alloccount(void)
	 {
		 return alloc_count;
	 }


	 //use ����  -  ����ڰ� ���� �����ִ� ����
	 int usecount(void)
	 {
		 return use_count;
	 }


	 int alloc_count;
	 int use_count;
	 Node* freenode;


};


*/

//
//
//
////=============================����=======================================//
///*---------------------------------------------------------------
//
//	procademy MemoryPool.
//
//�޸� Ǯ Ŭ���� (������Ʈ Ǯ / ��������Ʈ)
//Ư�� ����Ÿ(����ü,Ŭ����,����)�� ������ �Ҵ� �� ��������.
//
//- ����.
//
//procademy::CMemoryPool<DATA> MemPool(300, FALSE);
//	DATA *pData = MemPool.Alloc();
//
//	pData ���
//
//MemPool.Free(pData);
//
//
//----------------------------------------------------------------*/
//#ifndef  __PROCADEMY_MEMORY_POOL__
//#define  __PROCADEMY_MEMORY_POOL__
//#include <new.h>
//
//namespace procademy
//{
//
//	template <class DATA>
//	class CMemoryPool
//	{
//	private:
//
//		/* **************************************************************** */
//		// �� �� �տ� ���� ��� ����ü.
//		/* **************************************************************** */
//		struct st_BLOCK_NODE
//		{
//			st_BLOCK_NODE()
//			{
//				stpNextBlock = NULL;
//			}
//
//			st_BLOCK_NODE* stpNextBlock;
//		};
//
//
//
//
//	public:
//
//		//////////////////////////////////////////////////////////////////////////
//		// ������, �ı���.
//		//
//		// Parameters: (int) �ʱ� �� ����.
//		// (bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
//	// Return:
//		//////////////////////////////////////////////////////////////////////////
//		CMemoryPool(int iBlockNum, bool bPlacementNew = false);
//		virtual ~CMemoryPool();
//
//
//		//////////////////////////////////////////////////////////////////////////
//		// �� �ϳ��� �Ҵ�޴´�.  
//		//
//		// Parameters: ����.
//		// Return: (DATA *) ����Ÿ �� ������.
//		//////////////////////////////////////////////////////////////////////////
//		DATA* Alloc(void);
//
//		//////////////////////////////////////////////////////////////////////////
//		// ������̴� ���� �����Ѵ�.
//		//
//		// Parameters: (DATA *) �� ������.
//		// Return: (BOOL) TRUE, FALSE.
//		//////////////////////////////////////////////////////////////////////////
//		bool Free(DATA* pData);
//
//
//		//////////////////////////////////////////////////////////////////////////
//		// ���� Ȯ�� �� �� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
//		//
//		// Parameters: ����.
//		// Return: (int) �޸� Ǯ ���� ��ü ����
//	//////////////////////////////////////////////////////////////////////////
//		int GetAllocCount(void) { return m_iAllocCount; }
//
//		//////////////////////////////////////////////////////////////////////////
//		// ���� ������� �� ������ ��´�.
//		//
//		// Parameters: ����.
//		// Return: (int) ������� �� ����.
//		//////////////////////////////////////////////////////////////////////////
//		int GetUseCount(void) { return m_iUseCount; }
//
//
//		// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.
//
//		st_BLOCK_NODE* _pFreeNode;
//	};
//
//}
//#endif
//


#ifndef __MEMORY__POOL__
#define __MEMORY__POOL__



#define ALLOCSIZE 1000
#define CHKSUM 0x79ffff79

template <class Data>
class cMemoryPool
{
public:



	//������ �ı��� 
	//false �� ������ �ı��� ȣ���� �ȿ��� ����. 
	cMemoryPool(int poolsize, bool bcreatecall = false)
	{

		InitializeSRWLock(&_poolsrw);



		hHeap = HeapCreate(0, 0, 0);
		createcall = bcreatecall;
		alloc_count = poolsize;
		use_count = 0;

		if (alloc_count == 0) //����Ʈ�ǰ�� 
		{
		}
		
		Node* nextNode = nullptr;
		for (int i = 0; i < alloc_count; i++)
		{
			Node* node = (Node*)HeapAlloc(hHeap,0,sizeof(Node));
			if (i == 0)
			{
				endnode = node;
			}
			//heap ���� �޸𸮸� ��ұ� ������ Ŭ���� ������ ȣ��ȵ� 
			node->frontCHKsum = CHKSUM;
			node->endCHKsum = CHKSUM;
			node->Nextnode = nextNode;
			nextNode = node;
		}

		//�� ó�� ��� 
		freenode = nextNode;

	}

	virtual ~cMemoryPool(void)
	{
		HeapDestroy(hHeap);
	}

	//alloc
	Data* alloc(void)
	{
		AcquireSRWLockExclusive(&_poolsrw);

		cMassage::debugbuf[cMassage::debugcount] = 'L';
		if (InterlockedIncrement(&cMassage::debugcount) == 10000)
		{
			InterlockedExchange(&cMassage::debugcount,0);
		}
			


		Node* node = nullptr;
		//������ �� �����
		if (alloc_count == use_count)
		{
			/*Node* node = (Node*)HeapAlloc(hHeap, 0, sizeof(Node));
			node->frontCHKsum = CHKSUM;
			node->endCHKsum = CHKSUM;
			node->Nextnode = freenode;
			freenode = node;
			InterlockedIncrement(&alloc_count);*/

			node = (Node*)HeapAlloc(hHeap, 0, sizeof(Node));
			node->frontCHKsum = CHKSUM;
			node->endCHKsum = CHKSUM;
			node->Nextnode = freenode;
			new(&node->mydata) Data();
			freenode = node;
			InterlockedIncrement(&alloc_count);
		}

		Node* retnod = nullptr;
		//������ ȣ������ ���� �ľ��� ���� 


		retnod = freenode;
		freenode = retnod->Nextnode;

		if (createcall)
		{
			//������ ȣ�� 
			new(&retnod->mydata) Data();
		}

		InterlockedIncrement(&use_count);

		ReleaseSRWLockExclusive(&_poolsrw);

		cMassage::debugbuf[cMassage::debugcount] = 'R';
		if (InterlockedIncrement(&cMassage::debugcount) == 10000)
		{
			InterlockedExchange(&cMassage::debugcount, 0);
		}

		return &retnod->mydata;

	}
	// p = malloc(sizeof(Node) + sizeof(Data));

	//free
	BOOL free(Data* pData)
	{
		AcquireSRWLockExclusive(&_poolsrw);


		cMassage::debugbuf[cMassage::debugcount] = 'L';
		if (InterlockedIncrement(&cMassage::debugcount) == 10000)
		{
			InterlockedExchange(&cMassage::debugcount, 0);
		}

		Node* inNode = (Node*)((char*)pData - sizeof(unsigned int));


		//�������ִٸ� false
		if (use_count == 0)
		{
			ReleaseSRWLockExclusive(&_poolsrw);
			//���ܹ߻�
			return false;
		}
		if (inNode->endCHKsum != CHKSUM || inNode->frontCHKsum != CHKSUM)
		{
			ReleaseSRWLockExclusive(&_poolsrw);
			//���ܹ߻� 
			return false;
		}

		//�Ҹ��� ȣ�⿩��
		if (createcall)
		{
			//�Ҹ��� ȣ�� 
			inNode->mydata.~Data();
			//freenode->Nextnode = inNode;
			Node* savenode = freenode;
			freenode = inNode;
			freenode->Nextnode = savenode;

			//endnode->Nextnode = inNode;
			//inNode->Nextnode = nullptr;
			//endnode = inNode;
		}
		else
		{
			//�Ҹ��� x
			//freenode->Nextnode = inNode;
			Node* savenode = freenode;
			freenode = inNode;
			freenode->Nextnode = savenode;
			//endnode->Nextnode = inNode;
			//inNode->Nextnode = nullptr;
			//endnode = inNode;
		}
		InterlockedDecrement(&use_count);


		ReleaseSRWLockExclusive(&_poolsrw);

		cMassage::debugbuf[cMassage::debugcount] = 'R';
		if (InterlockedIncrement(&cMassage::debugcount) == 10000)
		{
			InterlockedExchange(&cMassage::debugcount, 0);
		}

		return true;

	}


	//alloc ���� - �Ҵ� �޾Ƶ� ���� 

	int alloccount(void)
	{
		return alloc_count;
	}


	//use ����  -  ����ڰ� ���� �����ִ� ����
	int usecount(void)
	{
		return use_count;
	}

private:

	//���� ����� ������
#pragma pack(1)
	struct Node
	{
		Node()
		{
			Nextnode = NULL;
		}
		//�����ڵ� �־�α� 
		unsigned int frontCHKsum;
		Data mydata;
		Node* Nextnode;
		unsigned int endCHKsum;
	};
#pragma pop

	LONG alloc_count;
	LONG use_count;
	Node* freenode;
	Node* endnode;
	BOOL createcall;
	HANDLE hHeap;

	SRWLOCK _poolsrw;



	// ���� ������� ��ȯ�� (�̻��) ������Ʈ ���� ����.

};


#endif // !__MEMORY__POOL__

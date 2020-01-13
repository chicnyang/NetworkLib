#pragma once
#include <new>

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

extern cDump dump;


template <class Data>
class cMemoryPool
{
public:



	//������ �ı��� 
	//false �� ������ �ı��� ȣ���� �ȿ��� ����. 
	cMemoryPool(int poolsize, bool bcreatecall = false)
	{
		hHeap = HeapCreate(0, 0, 0);
		createcall = bcreatecall;
		alloc_count = poolsize;
		use_count = 0;

		if (alloc_count == 0) //����Ʈ�ǰ�� 
		{
		}

		//Node* nextNode = nullptr;
		//for (int i = 0; i < alloc_count; i++)
		//{
		//	Node* node = (Node*)HeapAlloc(hHeap, 0, sizeof(Node));
		//	if (i == 0)
		//	{
		//		//endnode = node;
		//	}

		//	//heap ���� �޸𸮸� ��ұ� ������ Ŭ���� ������ ȣ��ȵ� 
		//	node->frontCHKsum = CHKSUM;
		//	node->endCHKsum = CHKSUM;
		//	node->Nextnode = nextNode;
		//	nextNode = node;
		//}

		//�� ó�� ��� 
		//freenode = nextNode;

		_stTop = (stTop*)_aligned_malloc(sizeof(stTop), 16);
		_stTop->_Top = nullptr;
		_stTop->popcount = 0;
	}

	virtual ~cMemoryPool(void)
	{
		while (_stTop->_Top != nullptr)
		{
			Node* deleteNode = _stTop->_Top;
			_stTop->_Top = deleteNode->Nextnode;
			deleteNode->mydata.~Data();
		}
		_aligned_free(_stTop);
		HeapDestroy(hHeap);
	}

	//alloc
	Data* alloc(void)
	{
		Node* node = nullptr;
		//������ �� �����
		Data* retdata;

		if (InterlockedDecrement(&free_count) < 0)
		{
			node = (Node*)HeapAlloc(hHeap, 0, sizeof(Node));
			node->frontCHKsum = CHKSUM;
			node->endCHKsum = CHKSUM;

			new(&node->mydata) Data();

			InterlockedIncrement(&alloc_count);
			InterlockedIncrement(&free_count);
			retdata = &node->mydata;
		}
		else
		{
			stTop topSrc;


			for (;;)
			{
				topSrc.popcount = _stTop->popcount;
				topSrc._Top = _stTop->_Top;
				//topSrc = *_stTop;
				//SwitchToThread();
				retdata = &topSrc._Top->mydata;
				if (InterlockedCompareExchange128((volatile LONG64*)_stTop, (LONG64)_stTop->_Top->Nextnode, 1 + _stTop->popcount, (LONG64*)&topSrc))
				{
					break;
				}
			}

			//retnod = topSrc._Top;
			////������ ȣ������ ���� �ľ��� ���� 
			//if (createcall)
			//{
			//	//������ ȣ�� 
			//	new(&retnod->mydata) Data();
			//}
		}
		//retnod->Nextnode = nullptr;
		InterlockedIncrement(&use_count);
	
		return retdata;
	}

	//free
	BOOL free(Data* pData)
	{
		//AcquireSRWLockExclusive(&_poolsrw);

		Node* inNode = (Node*)((char*)pData - sizeof(unsigned int));
		//�������ִٸ� false
		if (use_count == 0)
		{
			//���ܹ߻�
			dump.Crash();
			return false;
		}
		if (inNode->endCHKsum != CHKSUM || inNode->frontCHKsum != CHKSUM)
		{
			//���ܹ߻� 
			dump.Crash();
			return false;
		}
		//�Ҹ��� ȣ�⿩��
		//if (createcall)
		//{
		//	//�Ҹ��� ȣ�� 
		//	inNode->mydata.~Data();
		//	Node* savenode = freenode;
		//	freenode = inNode;
		//	freenode->Nextnode = savenode;
		//}
		//else
		//{
		//	
		//}
		stTop topSrc;



		for (;;)
		{
			topSrc.popcount = _stTop->popcount;
			topSrc._Top = _stTop->_Top;
			//topSrc = *_stTop;
			//SwitchToThread();
			inNode->Nextnode = topSrc._Top;

			if (InterlockedCompareExchange128((volatile LONG64*)_stTop, (LONG64)inNode, 1 + _stTop->popcount,(LONG64*)&topSrc))
			{
				break;
			}
		}

		InterlockedDecrement(&use_count);
		InterlockedIncrement(&free_count);
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
#pragma pack(push, 1)
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
#pragma pack(pop)

	LONG alloc_count;
	LONG use_count;
	LONG free_count;
	BOOL createcall;
	HANDLE hHeap;

	//128 �� ����ü 
	struct stTop
	{
		LONG64 popcount;
		Node* _Top;
	};

	stTop* _stTop;

};


#endif // !__MEMORY__POOL__


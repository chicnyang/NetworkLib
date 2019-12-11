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
//		// �� ���� �տ� ���� ��� ����ü.
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
//		// Parameters: (int) �ʱ� ���� ����.
//		// (bool) Alloc �� ������ / Free �� �ı��� ȣ�� ����
//	// Return:
//		//////////////////////////////////////////////////////////////////////////
//		CMemoryPool(int iBlockNum, bool bPlacementNew = false);
//		virtual ~CMemoryPool();
//
//
//		//////////////////////////////////////////////////////////////////////////
//		// ���� �ϳ��� �Ҵ�޴´�.  
//		//
//		// Parameters: ����.
//		// Return: (DATA *) ����Ÿ ���� ������.
//		//////////////////////////////////////////////////////////////////////////
//		DATA* Alloc(void);
//
//		//////////////////////////////////////////////////////////////////////////
//		// ������̴� ������ �����Ѵ�.
//		//
//		// Parameters: (DATA *) ���� ������.
//		// Return: (BOOL) TRUE, FALSE.
//		//////////////////////////////////////////////////////////////////////////
//		bool Free(DATA* pData);
//
//
//		//////////////////////////////////////////////////////////////////////////
//		// ���� Ȯ�� �� ���� ������ ��´�. (�޸�Ǯ ������ ��ü ����)
//		//
//		// Parameters: ����.
//		// Return: (int) �޸� Ǯ ���� ��ü ����
//	//////////////////////////////////////////////////////////////////////////
//		int GetAllocCount(void) { return m_iAllocCount; }
//
//		//////////////////////////////////////////////////////////////////////////
//		// ���� ������� ���� ������ ��´�.
//		//
//		// Parameters: ����.
//		// Return: (int) ������� ���� ����.
//		//////////////////////////////////////////////////////////////////////////
//		int GetUseCount(void) { return m_iUseCount; }
//
//
//		// ���� ������� ��ȯ�� (�̻��) ������Ʈ ������ ����.
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
		hHeap = HeapCreate(0, 0, 0);
		createcall = bcreatecall;
		alloc_count = poolsize;

		if (alloc_count == 0) //����Ʈ�ǰ�� 
		{
			freenode = nullptr;
			return;
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
			node.frontCHKsum = CHKSUM;
			node.endCHKsum = CHKSUM;
			node.Nextnode = nextNode;
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

		//������ �� �����
		if (alloc_count == use_count)
		{
			Node* node = (Node*)HeapAlloc(hHeap, 0, sizeof(Node));
			node.frontCHKsum = CHKSUM;
			node.endCHKsum = CHKSUM;
			node.Nextnode = freenode;
			freenode = node;
			InterlockedIncrement(&alloc_count);
		}

		Node* retnod = nullptr;
		//������ ȣ������ ���� �ľ��� ���� 
		if (createcall)
		{
			//������ ȣ�� 
			retnod = freenode;
			new(&retnod->mydata) Data();
			freenode = retnod->nextNode;
		}
		else
		{
			//������ȣ�� x
			freenode = retnod->nextNode;
		}
		InterlockedIncrement(&use_count);

		return &retnod->mydata;

	}
	// p = malloc(sizeof(Node) + sizeof(Data));

	//free
	BOOL free(Data* pData)
	{

		Node* inNode = ((char*)pData - sizeof(int));


		//�������ִٸ� false
		if (usecount == 0)
		{
			//���ܹ߻�
			return false;
		}
		if (inNode->endCHKsum != CHKSUM || inNode->frontCHKsum != CHKSUM)
		{
			//���ܹ߻� 
			return false;
		}

		//�Ҹ��� ȣ�⿩��
		if (createcall)
		{
			//�Ҹ��� ȣ�� 
			retnod->mydata.~Data();
			//freenode->nextNode = inNode;
			//freenode = inNode;
			endnode->nextNode = inNode;
			inNode->nextNode = nullptr;
			endnode = inNode;
		}
		else
		{
			//�Ҹ��� x
			//freenode->nextNode = inNode;
			//freenode = inNode;
			endnode->nextNode = inNode;
			inNode->nextNode = nullptr;
			endnode = inNode;
		}
		InterlockedDecrement(&use_count);

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
	struct Node
	{
		Node()
		{
			Nextnode = NULL;
		}
		//�����ڵ� �־�α� 
		int frontCHKsum;
		Data mydata;
		Node* Nextnode;
		int endCHKsum;
	};


	LONG alloc_count;
	LONG use_count;
	Node* freenode;
	Node* endnode;
	BOOL createcall;
	HANDLE hHeap;

	// ���� ������� ��ȯ�� (�̻��) ������Ʈ ������ ����.

};


#endif // !__MEMORY__POOL__
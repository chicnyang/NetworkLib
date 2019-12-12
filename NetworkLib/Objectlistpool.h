#pragma once
#include <new>
#include <Windows.h>


//namespace 쓸지말지는 선택 중복이름 없으면 상관없음.


/*
template <class Data>
class objectListPool
{
	//다음 노드의 포인터
	struct Node
	{
		Node()
		{
			Nextnode = NULL;
		}
		Node* Nextnode;
	};

	//생성자 파괴자 
	//false 는 생성자 파괴자 호출을 안에서 안함. 
	objectListPool(int poolsize, bool bcreatecall = false)
	{
		if (poolsize == 0) //리스트의경우 
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


	 //alloc 갯수 - 할당 받아둔 갯수 

	 int alloccount(void)
	 {
		 return alloc_count;
	 }


	 //use 갯수  -  사용자가 지금 쓰고있는 갯수
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
////=============================설명=======================================//
///*---------------------------------------------------------------
//
//	procademy MemoryPool.
//
//메모리 풀 클래스 (오브젝트 풀 / 프리리스트)
//특정 데이타(구조체,클래스,변수)를 일정량 할당 후 나눠쓴다.
//
//- 사용법.
//
//procademy::CMemoryPool<DATA> MemPool(300, FALSE);
//	DATA *pData = MemPool.Alloc();
//
//	pData 사용
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
//		// 각 블럭 앞에 사용될 노드 구조체.
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
//		// 생성자, 파괴자.
//		//
//		// Parameters: (int) 초기 블럭 개수.
//		// (bool) Alloc 시 생성자 / Free 시 파괴자 호출 여부
//	// Return:
//		//////////////////////////////////////////////////////////////////////////
//		CMemoryPool(int iBlockNum, bool bPlacementNew = false);
//		virtual ~CMemoryPool();
//
//
//		//////////////////////////////////////////////////////////////////////////
//		// 블럭 하나를 할당받는다.  
//		//
//		// Parameters: 없음.
//		// Return: (DATA *) 데이타 블럭 포인터.
//		//////////////////////////////////////////////////////////////////////////
//		DATA* Alloc(void);
//
//		//////////////////////////////////////////////////////////////////////////
//		// 사용중이던 블럭을 해제한다.
//		//
//		// Parameters: (DATA *) 블럭 포인터.
//		// Return: (BOOL) TRUE, FALSE.
//		//////////////////////////////////////////////////////////////////////////
//		bool Free(DATA* pData);
//
//
//		//////////////////////////////////////////////////////////////////////////
//		// 현재 확보 된 블럭 개수를 얻는다. (메모리풀 내부의 전체 개수)
//		//
//		// Parameters: 없음.
//		// Return: (int) 메모리 풀 내부 전체 개수
//	//////////////////////////////////////////////////////////////////////////
//		int GetAllocCount(void) { return m_iAllocCount; }
//
//		//////////////////////////////////////////////////////////////////////////
//		// 현재 사용중인 블럭 개수를 얻는다.
//		//
//		// Parameters: 없음.
//		// Return: (int) 사용중인 블럭 개수.
//		//////////////////////////////////////////////////////////////////////////
//		int GetUseCount(void) { return m_iUseCount; }
//
//
//		// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.
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



	//생성자 파괴자 
	//false 는 생성자 파괴자 호출을 안에서 안함. 
	cMemoryPool(int poolsize, bool bcreatecall = false)
	{

		InitializeSRWLock(&_poolsrw);



		hHeap = HeapCreate(0, 0, 0);
		createcall = bcreatecall;
		alloc_count = poolsize;
		use_count = 0;

		if (alloc_count == 0) //리스트의경우 
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
			//heap 으로 메모리만 잡았기 때문에 클래스 생성자 호출안됨 
			node->frontCHKsum = CHKSUM;
			node->endCHKsum = CHKSUM;
			node->Nextnode = nextNode;
			nextNode = node;
		}

		//맨 처음 노드 
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
		//없으면 더 만들기
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
		//생성자 호출할지 여부 파악후 리턴 


		retnod = freenode;
		freenode = retnod->Nextnode;

		if (createcall)
		{
			//생성자 호출 
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


		//가득차있다면 false
		if (use_count == 0)
		{
			ReleaseSRWLockExclusive(&_poolsrw);
			//예외발생
			return false;
		}
		if (inNode->endCHKsum != CHKSUM || inNode->frontCHKsum != CHKSUM)
		{
			ReleaseSRWLockExclusive(&_poolsrw);
			//예외발생 
			return false;
		}

		//소멸자 호출여부
		if (createcall)
		{
			//소멸자 호출 
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
			//소멸자 x
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


	//alloc 갯수 - 할당 받아둔 갯수 

	int alloccount(void)
	{
		return alloc_count;
	}


	//use 갯수  -  사용자가 지금 쓰고있는 갯수
	int usecount(void)
	{
		return use_count;
	}

private:

	//다음 노드의 포인터
#pragma pack(1)
	struct Node
	{
		Node()
		{
			Nextnode = NULL;
		}
		//구별코드 넣어두기 
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



	// 스택 방식으로 반환된 (미사용) 오브젝트 블럭을 관리.

};


#endif // !__MEMORY__POOL__

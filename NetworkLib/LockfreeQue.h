#pragma once

#ifndef __LOCK_FREE_QUE__
#define __LOCK_FREE_QUE__


extern cDump dump;

template<class DATA>
class LockfreeQue
{
public:
	LockfreeQue()
	{
		//처음에 더미노드 하나 있어야함. 
		_nodePool = new cMemoryPool<Node>(0);
		_stFront = new stNodeCount;
		_stTail = new stNodeCount;
		_stFront->nodePointer = _nodePool->alloc();
		_stFront->nodePointer->_data = NULL;
		_stFront->nodePointer->_nextNode = nullptr;
		_stFront->popcount = 0;

		_stTail->nodePointer = _stFront->nodePointer;
		_stTail->popcount = _stFront->popcount;
	}
	~LockfreeQue()
	{
		//delete _stFront->nodePointer;
		delete _stFront;
	}

	//Deque
	DATA Deque()
	{


		////큐사이즈 확인 
		int size = InterlockedDecrement(&quesize);
		if (size < 0)
		{
			InterlockedIncrement(&quesize);
			return NULL;
		}

		stNodeCount copyFront;
		DATA retdata = NULL;
		//디큐 하기전 tail 을 밀수 있다면 밀자 



		//if (_stTail->nodePointer == _stFront->nodePointer)
		//{
		//	stNodeCount copyTail;			

		//	copyTail.nodePointer = _stTail->nodePointer;
		//	copyTail.popcount = _stTail->popcount;

		//	Node* tailNext = copyTail.nodePointer->_nextNode;
		//	if (Next != nullptr)
		//	{
		//		//tail 변경시 count 필요 
		//		InterlockedCompareExchange128((volatile LONG64*)_stTail, 1 + _stTail->popcount, (LONG64)Next, (LONG64*)&copyTail);
		//	}
		//}



		for (;;)
		{

			copyFront.nodePointer = _stFront->nodePointer;
			copyFront.popcount = _stFront->popcount;

			Node* next = copyFront.nodePointer->_nextNode;

			if (next == nullptr)
			{
				//다른곳에서 새노드에 붙인후  size++ 되고 실제 tail 에 안붙은 상태 - 정상 
				continue;
			}
			else
			{
				stNodeCount copyTail;

				copyTail.nodePointer = _stTail->nodePointer;
				copyTail.popcount = _stTail->popcount;

				Node* tailNext = copyTail.nodePointer->_nextNode;
				if (tailNext != nullptr)
				{
					//tail 변경시 count 필요 
					InterlockedCompareExchange128((volatile LONG64*)_stTail, 1 + _stTail->popcount, (LONG64)tailNext, (LONG64*)&copyTail);
				}


				retdata = next->_data;
				if (InterlockedCompareExchange128((volatile LONG64*)_stFront, 1 + _stFront->popcount, (LONG64)next,(LONG64*)&copyFront))
				{
					break;
				}
			}

		}

		_nodePool->free(copyFront.nodePointer);
	//	InterlockedDecrement(&quesize);

		if (retdata == NULL)
		{
			dump.Crash();
			return NULL;
		}
		return retdata;
	}

	//Enque
	void Enque(DATA data)
	{

		//노드만들어서 붙이기 
		Node* newNode = _nodePool->alloc();
		newNode->_data = data;
		newNode->_nextNode = nullptr;

		if (data == NULL)
		{
			dump.Crash();
		}

		stNodeCount copyTail;
		Node* Next;

		for (;;)
		{
			//카피 테일의 next 와 진짜 tail 의  next 비교 

			copyTail.nodePointer = _stTail->nodePointer;
			copyTail.popcount = _stTail->popcount;


			Next = copyTail.nodePointer->_nextNode;

			if (Next == nullptr) //불필요한 compare 안함.
			{
				if ((LONG64)Next == InterlockedCompareExchange64((volatile LONG64*)&copyTail.nodePointer->_nextNode, (LONG64)newNode, (LONG64)Next))
				{
					InterlockedCompareExchange128((volatile LONG64*)_stTail, 1 + _stTail->popcount, (LONG64)newNode, (LONG64*)&copyTail);
					break;
				}
			}
			else
			{
				InterlockedCompareExchange128((volatile LONG64*)_stTail, 1 + _stTail->popcount, (LONG64)Next, (LONG64*)&copyTail);
			}
		}
		InterlockedIncrement(&quesize);
		return;
	}

	LONG quesize;

	struct Node
	{
		Node* _nextNode;
		DATA _data;
	};

	cMemoryPool<Node> *_nodePool;
private:

	//128 용 구조체 
	struct stNodeCount
	{

		Node* nodePointer;
		LONG64 popcount;
	};

	stNodeCount* _stFront;
	stNodeCount* _stTail;


	struct Debug
	{
		
	};

};

#endif // !__LOCK_FREE_QUE__




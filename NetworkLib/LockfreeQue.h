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
		//ó���� ���̳�� �ϳ� �־����. 
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


		////ť������ Ȯ�� 
		int size = InterlockedDecrement(&quesize);
		if (size < 0)
		{
			InterlockedIncrement(&quesize);
			return NULL;
		}

		stNodeCount copyFront;
		DATA retdata = NULL;
		//��ť �ϱ��� tail �� �м� �ִٸ� ���� 



		//if (_stTail->nodePointer == _stFront->nodePointer)
		//{
		//	stNodeCount copyTail;			

		//	copyTail.nodePointer = _stTail->nodePointer;
		//	copyTail.popcount = _stTail->popcount;

		//	Node* tailNext = copyTail.nodePointer->_nextNode;
		//	if (Next != nullptr)
		//	{
		//		//tail ����� count �ʿ� 
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
				//�ٸ������� ����忡 ������  size++ �ǰ� ���� tail �� �Ⱥ��� ���� - ���� 
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
					//tail ����� count �ʿ� 
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

		//��常�� ���̱� 
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
			//ī�� ������ next �� ��¥ tail ��  next �� 

			copyTail.nodePointer = _stTail->nodePointer;
			copyTail.popcount = _stTail->popcount;


			Next = copyTail.nodePointer->_nextNode;

			if (Next == nullptr) //���ʿ��� compare ����.
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

	//128 �� ����ü 
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




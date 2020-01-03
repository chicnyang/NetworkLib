#pragma once
#include <Windows.h>
#include "DumpFile.h"

#ifndef __LOCK_FREE_STACK__
#define __LOCK_FREE_STACK__

extern cDump dump;
//long cDump::dumpcount;


template<class DATA>
class LockfreeStack
{
public:
	LockfreeStack()
	{
		char* d = (char*)malloc(sizeof(char));

		//_stTop = (stTop*)malloc(sizeof(stTop));
		_stTop = (stTop*)_aligned_malloc(sizeof(stTop), 16);
		_stTop->_Top = nullptr;
		_stTop->popcount = 0;
		stacksize = 0;
	}
	~LockfreeStack()
	{
		//메모리 해제 
		while (_stTop->_Top != nullptr)
		{
			Node* NextTop = (_stTop->_Top)->_nextNode;
			delete _stTop->_Top;
			_stTop->_Top = NextTop;
		}

		_aligned_free(_stTop);
	}


	//push
	void Push(DATA data)
	{
		//노드만들어서 붙이기 
		Node* newNode = new Node;
		newNode->_data = data;


		stTop topSrc;



		for(;;)
		{
			topSrc = *_stTop;
			
			//topSrc._Top = _stTop->_Top;
			//topSrc.popcount = _stTop->popcount;


			newNode->_nextNode = _stTop->_Top;

			if (InterlockedCompareExchange128((volatile LONG64*)_stTop, (LONG64)newNode, _stTop->popcount, (LONG64*)&topSrc))
			{
				break;
			}
			//if ((LONG64)topSrc._Top == InterlockedCompareExchange64((volatile LONG64*)&_stTop->_Top,(LONG64)newNode, (LONG64)topSrc._Top))
			//{
			//	break;
			//}
		}
		InterlockedIncrement(&stacksize);
	}

	//pop
	DATA Pop()
	{
		//맨위의 노드 하나 지우고 데이터 넘기기 

		//InterlockedDecrement(&stacksize);
		if (InterlockedDecrement(&stacksize) < 0)
		{
			InterlockedIncrement(&stacksize);
			return NULL;
		}

		stTop topSrc;


		for (;;)
		{
			topSrc = *_stTop;
			//topSrc._Top = _stTop->_Top;
			//topSrc.popcount = _stTop->popcount;

			

			if (InterlockedCompareExchange128((volatile LONG64*)_stTop, (LONG64)_stTop->_Top->_nextNode, _stTop->popcount + 1, (LONG64*)&topSrc))
			{
				break;
			}
		}

		DATA retdata = (topSrc._Top)->_data;
		delete topSrc._Top;
		return retdata;
	}
	LONG stacksize;

private:

	//노드
	struct Node
	{
		DATA _data;
		Node* _nextNode;
	};

	//128 용 구조체 
	struct stTop 
	{
		LONG64 popcount;
		Node* _Top;

	};
	

	stTop* _stTop;


};

#endif // !__LOCK_FREE_STACK__




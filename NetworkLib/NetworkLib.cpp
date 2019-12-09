// NetworkLib.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"


extern cDump dump;

cNetworkLib::cNetworkLib()
{

}

cNetworkLib::~cNetworkLib()
{

}

void cNetworkLib::SendPacket(__int64 sessionKey, cMassage * packet)
{

	stSession* session = FindSession(sessionKey);
	if (session == NULL)
	{
		return;
	}
	


	EnterCriticalSection(&session->cs);

	if (session->type == release)
	{
		LeaveCriticalSection(&session->cs);
		return;
	}
	if (session->sessionKey != sessionKey)
	{
		LeaveCriticalSection(&session->cs);

		return;
	}

	WORD usesize = packet->Getusesize();
	//헤더넣고 페이로드 인큐 

	int enqsize = session->sendQ.Enque((BYTE*)&usesize,sizeof(WORD));
	int enqpaysize = session->sendQ.Enque(packet->Getbufferptr(), usesize);

	SendPost(session);

	LeaveCriticalSection(&session->cs);
}

void cNetworkLib::Disconnect(__int64 sessionKey)
{
	//일단 bclose 변수 확인후 false 면 closesocket 

	stSession* session = FindSession(sessionKey);

	if (session == NULL)
	{
		return;
	}

	EnterCriticalSection(&session->cs);

	if(InterlockedExchange((LONG*)&session->bClose, 1)==0)
	{
		SOCKET closesock = session->socket;
		session->socket = INVALID_SOCKET;
	//closesocket(closesock);
	}

	LeaveCriticalSection(&session->cs);
}

void cNetworkLib::StartNetserver(ServerSetting* serversetting)
{

	sessionNum = 1;


	poolCount = serversetting->SessionPoolCount;

	for (int i = 0; i < poolCount; i++)
	{
		stSession* session = new stSession;
		InitializeCriticalSection(&session->cs);

		session->bClose = 0;
		session->bSend = 0;
		ZeroMemory(&session->recvoverlap, sizeof(session->recvoverlap));
		ZeroMemory(&session->sendoverlap, sizeof(session->sendoverlap));
		session->recvoverlap.mode = recvMode;
		session->sendoverlap.mode = sendMode;

		session->type = release;
		sessionPool.push_back(session);
	}

	//초기 설정 // IP Port ..
	//서버 스타트-> 스레드 생성 
	serveraddr = serversetting->sockaddr;
	ThreadMax = serversetting->ThreadMax+1;
	ThreadRunCount = serversetting->ThreadRun;

	InitializeSRWLock(&map_cs);
	InitializeCriticalSection(&pool_cs);

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		//윈속 초기화 실패 
		return;
	}

	//IOCP 포트 생성 
	hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	ThreadArray = new HANDLE[ThreadMax];

	//소켓 만들고 바인딩 리슨 
	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == INVALID_SOCKET)
	{
		//소켓 생성실패 
		return;
	}

	int retbind = bind(listen_socket, (SOCKADDR*)&serveraddr, sizeof(serveraddr));
	if (retbind == SOCKET_ERROR)
	{
		//바인드 실패 
		wprintf_s(L"bind fail\n");
		closesocket(listen_socket);
		return;
	}

	//리슨 
	int retlisten = listen(listen_socket, SOMAXCONN);
	if (retlisten == SOCKET_ERROR)
	{
		//소켓 리슨실패 
		wprintf_s(L"listen fail\n");
		closesocket(listen_socket);
		return;
	}


	ThreadArray[0] = (HANDLE)_beginthreadex(NULL, 0, acceptTh, this, 0, NULL);
	for (int i = 1; i < ThreadMax; i++)
	{
		ThreadArray[i] = (HANDLE)_beginthreadex(NULL, 0, workerTh, this, 0, NULL);
	}

}

void cNetworkLib::CloseNetserver()
{
	//소켓 연결 끊기 - 리슨 소켓 클로즈
	closesocket(listen_socket);


	//리스트와 맵의 모든 요소 해제 
	for(;;)
	{
		if (sessionMap.empty() && sessionPool.size() == poolCount)
			break;
	}


	//pool 의 요소 하나씩 다 정리하기 

	std::list<stSession*>::iterator iter;
	std::list<stSession*>::iterator olditer = sessionPool.end();

	for (iter = sessionPool.begin(); iter != olditer;)
	{
		DeleteCriticalSection(&(*iter)->cs);
		delete *iter;
		iter = sessionPool.erase(iter);
	}

	//스레드 종료 
	for (int i = 0; i < ThreadMax; i++)
	{
		PostQueuedCompletionStatus(hIocp, NULL, NULL, NULL);
	}

	//스레드 종료 기다린다. 
	WaitForMultipleObjects(ThreadMax, ThreadArray, TRUE, INFINITE);

	//스레드 핸들 클로즈 
	for (int i = 0; i < ThreadMax; i++)
	{
		CloseHandle(ThreadArray[i]);
	}
	
	//스레드 배열 정리
	delete[] ThreadArray;

	DeleteCriticalSection(&pool_cs);



}

unsigned int __stdcall cNetworkLib::workerTh(LPVOID thisclass)
{

	cNetworkLib* cthisclass = (cNetworkLib*)thisclass;

	cthisclass->WorkerLoop();

	return 0;
}

unsigned int __stdcall cNetworkLib::acceptTh(LPVOID thisclass)
{
	cNetworkLib* cthisclass = (cNetworkLib*)thisclass;

	cthisclass->AcceptLoop();

	return 0;
}

//===============================Accept 를 호출하는 스레드 루프 
void cNetworkLib::AcceptLoop()
{
	for (;;)
	{
		SOCKET sock;
		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);  //size 넣어줄것...

		sock = accept(listen_socket, (SOCKADDR*)&clientaddr, &addrlen);
		if (sock == INVALID_SOCKET)
		{
			DWORD err = WSAGetLastError();
			//accept 실패 
			wprintf_s(L"accept fail  %d\n", err);
			closesocket(listen_socket);
			return;
		}

		char ip[20] = { 0 };
		inet_ntop(AF_INET, &(clientaddr.sin_addr), ip, 20);


		//세션정보 넣기 

		stSession* session = AllocSession();


		if (session == NULL)
		{
			wprintf_s(L"pool is full\n");
			return;
		}

		session->sessionKey = sessionNum;
		session->socket = sock;
		session->IOcount = 0;

		session->type = alloc;

		// 맵에 넣기 
		InputSession(session);  

		//소켓과 IOCP 연결 
		CreateIoCompletionPort((HANDLE)session->socket, hIocp, (ULONG_PTR)session, 0);

		if(RecvPost(session))
			onClientJoin(session->sessionKey);


		sessionNum++;
	}
}

void cNetworkLib::WorkerLoop()
{
	//recv 받으면 send 하고 또 recv 대기하는 스레드 
	//CreateIoCompletionPort((HANDLE)session_arr[account_num].sock, h_Iocp,(ULONG_PTR)&session_arr[account_num],0);
	for (;;)
	{
		stSession* mysession = NULL;
		DWORD transbyte = 0;
		Myoverlapped* overlap = NULL;

		do  //한번진행하고 빠져나감 
		{
			int retdebug = GetQueuedCompletionStatus(hIocp, &transbyte, (PULONG_PTR)&mysession, (LPOVERLAPPED*)&overlap, INFINITE);
			//overlapped 확인 

			if (overlap == NULL)
			{
				//iocp 문제 
				return;
			}

			//전송량 확인
			if (transbyte == 0)
			{
				//mysession->socket = INVALID_SOCKET;
				break;
			}
			if (overlap->mode == recvMode)  //recv 완료 통지일때만 로직 
			{
				//받은거 뽑아서 처리 
				int moverearresult = mysession->recvQ.Moverear(transbyte);
				if (moverearresult == 0 || moverearresult == -1)
				{
					LOG(L"ringbuffer",LOG_LEVEL_DEBUG,L"io - recv moverear  %d", transbyte);
				}

				for (;;)
				{
					WORD len = 0;
					int usesize = mysession->recvQ.Getquesize();

					if (usesize < sizeof(WORD))
					{
						//printf("헤더 크기 부족 ");
						break;
					}

					int peekresulr = mysession->recvQ.Peek((BYTE*)&len, sizeof(WORD));

					if (peekresulr == 0 || peekresulr == -1)
					{
						LOG(L"ringbuffer",LOG_LEVEL_DEBUG, L"io - recv peek  %d", usesize);
					}
					if (len != 8)
					{
						printf("잘못된 헤더 %d ", len);
					}

					if (usesize < sizeof(WORD) + len)  //메시지 완성안됨. 
					{
						printf("메시지 크기 부족 ");
						break;
					}

					cMassage msg;
					int header = mysession->recvQ.Deque((BYTE*)&len, sizeof(WORD));
					int deque = mysession->recvQ.Deque(msg.Getbufferptr(), len);

					if (header == 0 || header == -1)
					{
						LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"io - recv hederdeque  %d", usesize);
					}
					if (deque == 0 || deque == -1)
					{
						LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"io - recv hederdeque  %d", usesize);
					}
					msg.MoveWritepos(len);

					onRecv(mysession->sessionKey,&msg);
				}

				//recv 걸기
				RecvPost(mysession);
			}
			else  //send 일때 
			{
				int movefrontresult = mysession->sendQ.Movefront(transbyte);
				if (movefrontresult == 0 || movefrontresult == -1)
				{
					LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"io - send movefront  %d , result  %d", transbyte, movefrontresult);
				}
				//샌드 링버퍼에 보낼게 있다면보내기 
				InterlockedExchange((LONG*)&mysession->bSend, 0);
				int usesize = mysession->sendQ.Getquesize();

				if (usesize > 0)
				{
					SendPost(mysession);
				}
				else if (usesize < 0)
				{
					LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"io - send usesize -1  %d", usesize);
				}
			}

		} while (0);

		if (0 == InterlockedDecrement((LONG*)&mysession->IOcount))   //완료통지에 대한 차감 
		{
			DeleteSession(mysession);
		}
	}
}

BOOL cNetworkLib::RecvPost(stSession* session)
{
	DWORD flag = 0;
	WSABUF recvwsabuf[2];
	//wsabuf  에 받을 링버퍼 포인터 넣기 
	//링버퍼 경계 걸치는 지 확인후 추가로 버퍼포인터 넣을지 결정 


	int freesize = session->recvQ.Getquefreesize();
	int direnquesize = session->recvQ.DirectEnquesize();

	int recvbufcount = 0;

	if(direnquesize == 0 || freesize == 0)
	{
		LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"recvpost size 0 free -  %d , dir %d ", freesize,direnquesize);
	}

	recvwsabuf[0].buf = (char*)session->recvQ.Getrearptr();
	recvwsabuf[0].len = direnquesize;
	recvbufcount++;

	if (freesize > direnquesize)
	{
		recvwsabuf[1].buf = (char*)session->recvQ.Getbufptr();
		recvwsabuf[1].len = freesize - direnquesize;
		recvbufcount++;
	}

	ZeroMemory(&session->recvoverlap.overlap, sizeof(session->recvoverlap.overlap));




	DWORD retbyte = 0;


	InterlockedIncrement((LONG*)&session->IOcount);
	int retrecv = WSARecv(session->socket, recvwsabuf, recvbufcount, &retbyte, &flag, (OVERLAPPED*)&session->recvoverlap, NULL);
	if (retrecv == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			//오류 
			if (0 == InterlockedDecrement((LONG*)&session->IOcount))
			{
				DeleteSession(session);
				return false;
			}
			return false;
		}
	}

	return true;
}

void cNetworkLib::SendPost(stSession* session)
{

	//send post 호출은 IOcount 물고 들어오거나 
	//lock 을 걸고 들어 오기때문에 sessionkey 에 대한걸 저장할 필요없음

	int usesize;

	do
	{
		if (InterlockedExchange((LONG*)&session->bSend, 1) == 0)
		{

			usesize = session->sendQ.Getquesize();
		}
		else
			return;

		if (usesize == 0)
		{
			InterlockedExchange((LONG*)&session->bSend, 0);

			usesize = session->sendQ.Getquesize();
			if (usesize > 0)
				continue;
			else
				return;
		}
		else
			break;

	} while (0);


	WSABUF sendwsabuf[2];
	//wsabuf  에 받을 링버퍼 포인터 넣기 
	//링버퍼 경계 걸치는 지 확인후 추가로 버퍼포인터 넣을지 결정 

	int dirdequesize = session->sendQ.DirectDequesize();

	if (dirdequesize == 0 || usesize == 0)
	{
		LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"recvpost size 0 free -  %d , dir %d ", usesize, dirdequesize);
	}


	int sendbufcount = 0;


	if (usesize > dirdequesize)
	{
		sendwsabuf[0].buf = (char*)session->sendQ.Getfrontptr();
		sendwsabuf[0].len = dirdequesize;
		sendwsabuf[1].buf = (char*)session->sendQ.Getbufptr();
		sendwsabuf[1].len = usesize - dirdequesize;

		sendbufcount = 2;
	}
	else
	{
		sendwsabuf[0].buf = (char*)session->sendQ.Getfrontptr();
		sendwsabuf[0].len = dirdequesize;
		sendbufcount = 1;

	}

	ZeroMemory(&session->sendoverlap.overlap, sizeof(session->sendoverlap.overlap));
	DWORD sendretbyte = 0;


	if(InterlockedIncrement((LONG*)&session->IOcount) == 1)
	{
		InterlockedDecrement((LONG*)&session->IOcount);
		return;
	}

	int retsend = WSASend(session->socket, sendwsabuf, sendbufcount, &sendretbyte, 0, (OVERLAPPED*)&session->sendoverlap, NULL);
	if (retsend == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			//오류 
			if (0 == InterlockedDecrement((LONG*)&session->IOcount)) 
			{
				DeleteSession(session);
			}

			InterlockedExchange((LONG*)&session->bSend, 0);
			return;
		}

	}

	return;
}

cNetworkLib::stSession* cNetworkLib::AllocSession()
{
	stSession* session = NULL;

	//pool 에 대한 동기화 
	EnterCriticalSection(&pool_cs);

	if(!sessionPool.empty())
	{
		session = *(sessionPool.begin());
		sessionPool.pop_front();
	}
	else
	{
		LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"pool is full ");
	}
	LeaveCriticalSection(&pool_cs);

	return session;
}

void cNetworkLib::ReleaseSession(stSession * session)
{

	//pool 에 대한 동기화 
	EnterCriticalSection(&pool_cs);

	sessionPool.push_back(session);

	LeaveCriticalSection(&pool_cs);

}

void cNetworkLib::InputSession(stSession * session)
{
	AcquireSRWLockExclusive(&map_cs);

	sessionMap.insert(std::make_pair(session->sessionKey, session));

	ReleaseSRWLockExclusive(&map_cs);
}

void cNetworkLib::DeleteSession(stSession * session)
{
	stSession* deletesession = FindSession(session->sessionKey);
	if (deletesession == NULL)
	{
		LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"deletesession id -  %d ", session->sessionKey);
	}

	AcquireSRWLockExclusive(&map_cs);
	sessionMap.erase(session->sessionKey);
	ReleaseSRWLockExclusive(&map_cs);
	
	EnterCriticalSection(&session->cs);

	if (session->type == release)
	{
		LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"deletesession - release id -  %d ", session->sessionKey);
		LeaveCriticalSection(&session->cs);

		return;
	}

	SOCKET sock = session->socket;
	session->socket = INVALID_SOCKET;
	closesocket(sock);

	session->sessionKey = 0;
	session->bSend = 0;

	session->sendQ.Clearbuf();
	session->recvQ.Clearbuf();

	session->type = release;

	LeaveCriticalSection(&session->cs);

	ReleaseSession(session);

}

cNetworkLib::stSession* cNetworkLib::FindSession(__int64 sessionKey)
{

	stSession* session = NULL;

	std::unordered_map<__int64, stSession*>::iterator iter;

	AcquireSRWLockShared(&map_cs);

	iter = sessionMap.find(sessionKey);
	
	if (iter != sessionMap.end())
		session = (*iter).second;

	ReleaseSRWLockShared(&map_cs);

	return session;
}

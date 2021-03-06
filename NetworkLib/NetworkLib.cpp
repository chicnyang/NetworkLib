﻿// NetworkLib.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include "pch.h"

long cDump::dumpcount;
cDump dump;


cNetworkLib::cNetworkLib()
{

}

cNetworkLib::~cNetworkLib()
{

}

void cNetworkLib::SendPacket(__int64 sessionKey, cMassage * packet)
{

	if (packet->refcount != 1)
	{
		dump.Crash();
	}


	packet->settingHeader(2);





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

	//헤더 생성 

	//메시지 인큐 
	packet->refcntUp();

	if (session->sendQ.Enque((BYTE*)&packet, sizeof(packet)) == -1)
	{
		//인큐 실패 -> 세션 클로즈 해야함. 
		LeaveCriticalSection(&session->cs);
		CancelSession(session);
		packet->Free();
		//cancleio
		return;
	}



	packet->sendflag = true;


	SendPost(session);

	LeaveCriticalSection(&session->cs);
}

void cNetworkLib::Disconnect(__int64 sessionKey)
{
	//일단 bclose 변수 확인후 false 면 closesocket 

	stSession* session = FindSession(sessionKey);

	if (session == NULL)
	{
		dump.Crash();
		return;
	}
	CancelSession(session);

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

	WaitForSingleObject(ThreadArray[0],INFINITE);

	AcquireSRWLockExclusive(&map_cs);

	std::unordered_map<__int64, stSession*>::iterator mapiter;
	std::unordered_map<__int64, stSession*>::iterator mapenditer = sessionMap.end();
	for (mapiter = sessionMap.begin(); mapiter != mapenditer; ++mapiter)
	{
		closesocket((*mapiter).second->socket);
	}

	ReleaseSRWLockExclusive(&map_cs);


	//pool 에 다 들어올때까지 기다리기 

	for(;;)
	{
		if (sessionMap.empty() && sessionPool.size() == poolCount)
		{
			break;
		}

	}


	//pool 의 요소 하나씩 다 정리하기 

	std::list<stSession*>::iterator iter;
	std::list<stSession*>::iterator olditer = sessionPool.end();

	for (iter = sessionPool.begin(); iter != olditer;)
	{
		DeleteCriticalSection(&(*iter)->cs);

		int usesize = (*iter)->sendQ.Getquesize();
		if (usesize > 0)
		{
			for (int i = 0; i < (usesize / 8); i++)
			{
				cMassage* msg;
				(*iter)->sendQ.Deque((BYTE*)&msg,sizeof(msg));
				msg->Free();

			}
		}

		delete *iter;
		iter = sessionPool.erase(iter);
	}
	sessionMap.clear();
	sessionPool.clear();
	std::unordered_map<__int64, stSession*> emptysessionMap;
	std::list<stSession*> emptysessionPool;

	sessionMap.swap(emptysessionMap);
	sessionPool.swap(emptysessionPool);


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
	
	CloseHandle(hIocp);

	//스레드 배열 정리
	delete[] ThreadArray;

	DeleteCriticalSection(&pool_cs);


	WSACleanup();

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

		int optval = 0;
		//setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(const char*)&optval,sizeof(optval));

		AcceptCount++;
		AcceptTotalCount++;

		//1초 지나면 0으로 초기화 

		char ip[20] = { 0 };
		inet_ntop(AF_INET, &(clientaddr.sin_addr), ip, 20);

	/*	if (!OnConnectionRequest(ip, ntohs(clientaddr.sin_port)))
		{
			closesocket(sock);
		}*/

		//세션정보 넣기 

		stSession* session = AllocSession();


		if (session == NULL)
		{
			wprintf_s(L"pool is full %zd \n",sessionMap.size());
			return;
		}

		session->sessionKey = sessionNum;
		session->socket = sock;
		session->closesock = sock;
		session->IOcount = 0;

		session->type = alloc;

		// 맵에 넣기 
		InputSession(session);  

		//소켓과 IOCP 연결 
		CreateIoCompletionPort((HANDLE)session->socket, hIocp, (ULONG_PTR)session, 0);

		if (RecvPost(session))
		{
			onClientJoin(session->sessionKey);
		}



		sessionNum++;

	}
}


//======================================================= 워커 스레드 ===========================================================// 
void cNetworkLib::WorkerLoop()
{
	for (;;)
	{
		stSession* mysession = NULL;
		DWORD transbyte = 0;
		Myoverlapped* overlap = NULL;

		do  //한번진행하고 빠져나감 
		{
			int retdebug = GetQueuedCompletionStatus(hIocp, &transbyte, (PULONG_PTR)&mysession, (LPOVERLAPPED*)&overlap, INFINITE);
			//overlapped 확인 
			if (overlap->mode == sendMode)
			{
				DWORD endtime = timeGetTime();
				if ((endtime - mysession->startsend) < 2)
				{
					Plustime += (timeGetTime() - mysession->startsend);
					InterlockedIncrement(&sendcounttime);
				}
			}

			if (overlap == NULL)
			{
				//iocp 문제 
				return;
			}

			//전송량 확인
			if (transbyte == 0)
			{
				//dump.Crash();
				//mysession->socket = INVALID_SOCKET;
				break;
			}
			if (overlap->mode == recvMode)  //recv 완료 통지일때만 로직 
			{
				//받은거 뽑아서 처리 
				if (mysession->recvQ.Moverear(transbyte) == -1)
				{
					dump.Crash();
					LOG(L"ringbuffer",LOG_LEVEL_DEBUG,L"io - recv moverear  %d", transbyte);
				}
				int recvpacketcount = 0;
				cMassage* msg = cMassage::Alloc();

				for (;;)
				{
					msg->Clear();
					WORD len = 0;
					int usesize = mysession->recvQ.Getquesize();

					if (usesize < sizeof(WORD))
					{
						//printf("헤더 크기 부족 ");
						break;
					}

					if (mysession->recvQ.Peek((BYTE*)&len, sizeof(WORD)) == -1)
					{
						LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"io - recv peek  %d", usesize);
						dump.Crash();
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

					//cMassage msg;

					if (mysession->recvQ.Deque((BYTE*)&len, sizeof(WORD)) == -1)
					{
						LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"io - recv deque  %d", usesize);
						dump.Crash();
					}
					if (mysession->recvQ.Deque(msg->Getbufferptr(), len) == -1)
					{
						LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"io - recv deque  %d", usesize);
						dump.Crash();
					}

					msg->MoveWritepos(len);
					onRecv(mysession->sessionKey,msg);

					recvpacketcount++;
				}


				msg->Free();
				InterlockedAdd(&RecvCount, recvpacketcount);

				//recv 걸기
				RecvPost(mysession);
			}
			else  //send 일때 
			{
				//mysession->sendQ.Movefront(transbyte);
				int sendcount = mysession->sendQ.getsendcount();
				for (int i = 0; i < sendcount; i++)
				{
					cMassage* packet;
					if (mysession->sendQ.Deque((BYTE*)&packet, sizeof(packet)) == -1)
					{
						CancelSession(mysession);
						LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"io - send trans  %d", transbyte);
						break;
					}
					else
					{
						packet->Free();
					}
				}

				mysession->sendQ.setsendcount(0);
				InterlockedAdd(&SendCount, sendcount);

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
					dump.Crash();
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
		dump.Crash();
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

			if (usesize == 0)
			{
				InterlockedExchange((LONG*)&session->bSend, 0);

				usesize = session->sendQ.Getquesize();
				if (usesize > 0)
				{
					continue;
				}

				else
				{
					break;;
				}

			}
			else
			{
				//최대크기 
				WSABUF sendwsabuf[1000];
				//wsabuf  에 받을 링버퍼 포인터 넣기 
				//링버퍼 경계 걸치는 지 확인후 추가로 버퍼포인터 넣을지 결정 

				int dirdequesize = session->sendQ.DirectDequesize();

				if (dirdequesize == 0 || usesize == 0)
				{
					LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"recvpost size 0 free -  %d , dir %d ", usesize, dirdequesize);
				}


				int sendbufcount = usesize / 8;

				for (int i = 0; i < sendbufcount; i++)
				{
					cMassage* msg;
					if (-1 != session->sendQ.NextPeek((BYTE*)&msg, 8, sendbufcount))
					{
						sendwsabuf[i].buf = (char*)msg->GetHeaderbufferptr();
						sendwsabuf[i].len = msg->GetHeaderusesize();
						if (sendwsabuf[i].len == 0)
							dump.Crash();
					}
					else
					{
						sendbufcount = i;
					}
				}
				session->sendQ.setsendcount(sendbufcount);



				ZeroMemory(&session->sendoverlap.overlap, sizeof(session->sendoverlap.overlap));
				DWORD sendretbyte = 0;
				session->sendoverlap.sendcount = sendbufcount;

				if (InterlockedIncrement((LONG*)&session->IOcount) == 1)
				{
					InterlockedDecrement((LONG*)&session->IOcount);
					break;
				}

				session->startsend = timeGetTime();
				int retsend = WSASend(session->socket, sendwsabuf, sendbufcount, &sendretbyte, 0, (OVERLAPPED*)&session->sendoverlap, NULL);
				DWORD endtime = timeGetTime();
				if ((endtime - session->startsend) < 2)
				{
					sendPlustime += (endtime - session->startsend);
					InterlockedIncrement(&sendpluscounttime);
				}

				
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
						break;
					}

				}


			}

		
		}
		else
		{
			break;
		}


	} while (0);

	
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
	InterlockedIncrement(&ConnectSessioncount);
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
	InterlockedDecrement(&ConnectSessioncount);
	ReleaseSRWLockExclusive(&map_cs);
	
	EnterCriticalSection(&session->cs);

	if (session->type == release)
	{
		LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"deletesession - release id -  %d ", session->sessionKey);
		LeaveCriticalSection(&session->cs);

		return;
	}

	int sendcount = session->sendQ.Getquesize()/8;
	for (int i = 0; i < sendcount; i++)
	{
		cMassage* packet;
		session->sendQ.Deque((BYTE*)&packet, sizeof(packet));
		packet->Free();
	}


	session->socket = INVALID_SOCKET;
	closesocket(session->closesock);

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

void cNetworkLib::CancelSession(stSession * session)
{
	EnterCriticalSection(&session->cs);

	if (InterlockedExchange((LONG*)&session->bClose, 1) == 0)
	{
		session->socket = INVALID_SOCKET;
		CancelIo((HANDLE)session->closesock);
	}

	LeaveCriticalSection(&session->cs);
}

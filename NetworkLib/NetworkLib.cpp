// NetworkLib.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
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

void cNetworkLib::StartNetserver(ServerSetting* serversetting)
{

	sessionNum = 1;

	poolCount = serversetting->SessionPoolCount;
	sessionPool = new stSession[poolCount];

	for (WORD i = 0; i < poolCount; i++)
	{
		sessionPool[i].sendBufstack = new LockfreeQue<cMassage*>;
		sessionPool[i].sendQ = new LockfreeQue<cMassage*>;
		sessionPool[i].bClose = 0;
		sessionPool[i].bSend = 0;
		sessionPool[i].refCnt.bRelease = 1;
		sessionPool[i].refCnt.IOcount = 0;
		ZeroMemory(&sessionPool[i].recvoverlap, sizeof(sessionPool[i].recvoverlap));
		ZeroMemory(&sessionPool[i].sendoverlap, sizeof(sessionPool[i].sendoverlap));
		sessionPool[i].recvoverlap.mode = recvMode;
		sessionPool[i].sendoverlap.mode = sendMode;
		sessionPool[i].ArrayIndex = i;
		sessionPool[i].type = release;

		BlankIndexStack.Push(i);
	}

	//초기 설정 // IP Port ..
	//서버 스타트-> 스레드 생성 
	serveraddr = serversetting->sockaddr;
	ThreadMax = serversetting->ThreadMax+1;
	ThreadRunCount = serversetting->ThreadRun;

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

	//pool 에 다 들어올때까지 기다리기 

	for(;;)
	{
		if (BlankIndexStack.stacksize == poolCount)
		{
			break;
		}

	}

	//pool 의 요소 하나씩 다 정리하기 

	for (int i = 0; i < poolCount; i++)
	{
		int usesize = sessionPool[i].sendQ->quesize;
		if (usesize > 0)
		{
			for (int i = 0; i < (usesize / 8); i++)
			{
				cMassage* msg;
				msg = sessionPool[i].sendQ->Deque();
				msg->Free();

			}
		}
	}

	delete[] sessionPool;

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
			wprintf_s(L"pool is full \n");
			return;
		}

		InterlockedIncrement64(&session->refCnt.IOcount);

		session->sessionKey = sessionNum<<16;
		__int64* key = &session->sessionKey;
		*((WORD*)key) = session->ArrayIndex;


		session->socket = sock;
		session->closesock = sock;

		session->type = alloc;

		// 맵에 넣기 
		InputSession(session);  


		//for(;;)
		//{
		//	stRelease srcrelease;
		//	srcrelease.bRelease = 1;
		//	srcrelease.IOcount = 0;

		//	stRelease Exrelease;
		//	Exrelease.bRelease = 0;
		//	Exrelease.IOcount = 1;

		//	if (InterlockedCompareExchange128((volatile LONG64*)&(session->refCnt), (LONG64)Exrelease.IOcount, (LONG64)Exrelease.bRelease, (LONG64*)&srcrelease))
		//	{
		//		break;
		//	}

		//}




		//소켓과 IOCP 연결 
		CreateIoCompletionPort((HANDLE)session->socket, hIocp, (ULONG_PTR)session, 0);

		if (RecvPost(session))
		{
			onClientJoin(session->sessionKey);
		}

		session->refCnt.bRelease = 0;

		if (InterlockedDecrement64(&session->refCnt.IOcount) == 0)
		{
			DeleteSession(session);
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
				if (overlap->mode == sendMode)
					dump.Crash();
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
				//직렬화버퍼 스택에서 하나씩 빼서 delete
				//send 완료 통지에서 나만 건드림

				int sendcount = mysession->sendCount;

				for (int i = 0; i < sendcount; i++)
				{
					cMassage* packet = NULL;
					packet = mysession->sendBufstack->Deque();

					if (packet == NULL)
					{
						CancelSession(mysession);
						LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"io - send trans  %d , count %d", transbyte, sendcount);
						break;
					}
					else
					{
						packet->Free();
						InterlockedDecrement(&mysession->sendCount);
					}
				}


				InterlockedAdd(&SendCount, sendcount);

				//샌드 링버퍼에 보낼게 있다면보내기 
				InterlockedExchange((LONG*)&mysession->bSend, 0);
				int usesize = mysession->sendQ->quesize;

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

		if (0 == InterlockedDecrement64(&mysession->refCnt.IOcount))   //완료통지에 대한 차감 
		{
			DeleteSession(mysession);
		}
	}
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

	//세션 iocount 증가
	if (InterlockedIncrement64(&session->refCnt.IOcount) == 1)
	{
		if (0 == InterlockedDecrement64(&session->refCnt.IOcount))
		{
			DeleteSession(session);
		}
		return;
	}

	// iocount 가 1이아닌데 release 탄경우 예방
	if (session->refCnt.bRelease)
	{
		if (0 == InterlockedDecrement64(&session->refCnt.IOcount))
		{
			DeleteSession(session);
		}
		return;
	}

	//세션 id 끼리 확인 
	if (session->sessionKey != sessionKey)
	{
		if (0 == InterlockedDecrement64(&session->refCnt.IOcount))
		{
			DeleteSession(session);
		}
		return;
	}

	//헤더 생성 

	//메시지 인큐 
	packet->refcntUp();

	session->sendQ->Enque(packet);

	packet->sendflag = true;

	SendPost(session);


	if (0 == InterlockedDecrement64(&session->refCnt.IOcount))
	{
		DeleteSession(session);
	}
	return;
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


	InterlockedIncrement64(&session->refCnt.IOcount);
	int retrecv = WSARecv(session->socket, recvwsabuf, recvbufcount, &retbyte, &flag, (OVERLAPPED*)&session->recvoverlap, NULL);
	if (retrecv == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING)
		{
			//오류 
			if (0 == InterlockedDecrement64(&session->refCnt.IOcount))
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



	for (;;)
	{
		if (InterlockedExchange((LONG*)&session->bSend, 1) == 0)
		{
			usesize = session->sendQ->quesize;

			if (usesize == 0)
			{
				InterlockedExchange((LONG*)&session->bSend, 0);

				usesize = session->sendQ->quesize;
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

				usesize = session->sendQ->quesize;

				if (usesize == 0)
				{
					LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"recvpost size 0 free -  %d , dir %d ", usesize);
				}

				for (int i = 0; i < usesize; i++)
				{
					cMassage* msg;

					msg = session->sendQ->Deque();

					if (msg != NULL)
					{
						sendwsabuf[i].buf = (char*)msg->GetHeaderbufferptr();
						sendwsabuf[i].len = msg->GetHeaderusesize();
						if (sendwsabuf[i].len == 0)
							dump.Crash();


						session->sendBufstack->Enque(msg);
						InterlockedIncrement(&session->sendCount);
					}
				}

				ZeroMemory(&session->sendoverlap.overlap, sizeof(session->sendoverlap.overlap));
				DWORD sendretbyte = 0;
				session->sendoverlap.sendcount = usesize;

				InterlockedIncrement64(&session->refCnt.IOcount);
				//session->startsend = timeGetTime();
				int retsend = WSASend(session->socket, sendwsabuf, usesize, &sendretbyte, 0, (OVERLAPPED*)&session->sendoverlap, NULL);
				//DWORD endtime = timeGetTime();
				//if ((endtime - session->startsend) < 2)
				//{
				//	sendPlustime += (endtime - session->startsend);
				//	InterlockedIncrement(&sendpluscounttime);
				//}


				if (retsend == SOCKET_ERROR)
				{
					if (WSAGetLastError() != WSA_IO_PENDING)
					{
						//오류 
						if (0 == InterlockedDecrement64(&session->refCnt.IOcount))
						{
							DeleteSession(session);
						}

						InterlockedExchange(&session->bSend, 0);
						break;
					}

				}


			}


		}
		else
		{
			break;
		}


		break;
	}


	return;
}

cNetworkLib::stSession* cNetworkLib::AllocSession()
{
	stSession* session = NULL;

	//pool 에 대한 동기화 


	if(BlankIndexStack.stacksize > 0)
	{
		WORD index = BlankIndexStack.Pop();
		session = &sessionPool[index];
		if (session != NULL)
			session->refCnt.bRelease = 1;
	}
	else
	{
		LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"pool is full ");
	}
	return session;
}

void cNetworkLib::ReleaseSession(stSession * session)
{

	//pool 에 대한 동기화 
	BlankIndexStack.Push(session->ArrayIndex);

}

void cNetworkLib::InputSession(stSession * session)
{

	InterlockedIncrement(&ConnectSessioncount);
}

void cNetworkLib::DeleteSession(stSession * session)
{
	stSession* deletesession = FindSession(session->sessionKey);
	if (deletesession == NULL)
	{
		LOG(L"ringbuffer", LOG_LEVEL_DEBUG, L"deletesession id -  %d ", session->sessionKey);
	}

	InterlockedDecrement(&ConnectSessioncount);
	
	//release 플래그와  iocount 한번에 확인하기 

	stRelease srcrelease;
	srcrelease.bRelease = 0;
	srcrelease.IOcount = 0;

	stRelease Exrelease;
	Exrelease.bRelease = 1;
	Exrelease.IOcount = 0;

	if (InterlockedCompareExchange128((volatile LONG64*)&session->refCnt, (LONG64)Exrelease.IOcount, (LONG64)Exrelease.bRelease, (LONG64*)&srcrelease))
	{
		return;
	}

	int sendcount = session->sendQ->quesize;
	for (int i = 0; i < sendcount; i++)
	{
		cMassage* packet;
		packet = session->sendQ->Deque();
		if(packet != NULL)
			packet->Free();
	}

	int bufcount = session->sendBufstack->quesize;
	for (int i = 0; i < bufcount; i++)
	{
		cMassage* packet;
		packet = session->sendBufstack->Deque();
		if (packet != NULL)
			packet->Free();
	}

	session->socket = INVALID_SOCKET;
	closesocket(session->closesock);
	session->bSend = 0;
	session->recvQ.Clearbuf();
	session->type = release;

	ReleaseSession(session);

}

cNetworkLib::stSession* cNetworkLib::FindSession(__int64 sessionKey)
{

	stSession* session = NULL;

	WORD index = (WORD)sessionKey;
	if (sessionPool[index].type == alloc)
	{
		session = &sessionPool[index];
	}
	return session;
}

void cNetworkLib::CancelSession(stSession * session)
{
	if (InterlockedExchange((LONG*)&session->bClose, 1) == 0)
	{
		session->socket = INVALID_SOCKET;
		CancelIo((HANDLE)session->closesock);
	}
}

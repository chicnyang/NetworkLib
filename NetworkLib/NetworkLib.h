#pragma once

class cNetworkLib
{
public:
	cNetworkLib();
	virtual ~cNetworkLib();

	//server initail 세팅 구조체
	// 서버 IP Port  정보 / 워커 스레드 최대갯수 / 러닝 스레드 갯수 / 세션풀 갯수 
	struct ServerSetting
	{
		SOCKADDR_IN sockaddr;
		int ThreadMax;
		int ThreadRun;
		int SessionPoolCount;
	};
	

	//컨텐츠에서 호출할 함수들
	void SendPacket(__int64 sessionKey, cMassage* packet);
	void Disconnect(__int64 sessionKey);

	//서버 시작 / 서버 종료 

	void StartNetserver(ServerSetting* serversetting);
	void CloseNetserver();


	//서버 상황 변수

	//ConnectSession:
	LONG ConnectSessioncount;
	//Accept TPS :
	LONG AcceptCount;
	//Accept Total :
	__int64 AcceptTotalCount;
	//RecvPacket TPS :
	LONG RecvCount;
	//SendPacket TPS :
	LONG SendCount;

	LONG sendcounttime = 0;
	LONG sendpluscounttime = 0;
	__int64 Plustime = 0;

	__int64 sendPlustime = 0;


protected:

	enum Mode
	{
		sendMode = 0,
		recvMode = 1
	};
	enum Type
	{
		release = 0,
		alloc = 1
	};
	enum Buffersize
	{
		bufsize = 512
	};

	//overlapped
	struct Myoverlapped
	{
		WSAOVERLAPPED overlap;
		BYTE mode; //send 인지 recv 인지 구별
		int sendcount;
	};


	struct stRelease
	{
		LONG64 bRelease;
		LONG64 IOcount;
	};
	//session
	struct stSession
	{
		SOCKADDR_IN sockaddr;
		SOCKET		socket;
		SOCKET		closesock;

		__int64		sessionKey;

		Myoverlapped sendoverlap;
		Myoverlapped recvoverlap;

		LockfreeQue<cMassage*>* sendQ; //sendQ
		cRingbuffer recvQ;	//recvQ


		LockfreeQue<cMassage*>* sendBufstack; //send 한 직렬화버퍼 담는 스택 
		LONG sendCount;		//Send 직렬화버퍼 카운트 

		LONG bSend;			//샌드 플래그 
		LONG bClose;		//종료 플래그 


		stRelease refCnt;
		//LONG bRelease;		//릴리즈 플래그 
		//LONG IOcount;		//참조 카운트 


		DWORD type;			//사용 플래그

		DWORD startsend = 0; 

		WORD ArrayIndex;	//인덱스 번호 


	};

	virtual void onRecv(__int64 sessionKey, cMassage* msg) PURE;
	virtual void onClientJoin(__int64 sessionKey) PURE;
	virtual void onClientLeave() PURE;
	//virtual bool OnConnectionRequest(const char* clientIP,int Port) PURE;

private:


	//=========== IOCP 핸들
	HANDLE hIocp;

	//=========== 스레드 핸들 배열
	HANDLE* ThreadArray;

	//=========== 서버 ADDR
	SOCKADDR_IN serveraddr;

	//=========== listen 소켓
	SOCKET listen_socket;

	//=========== 세션 넘버 +1
	__int64 sessionNum;

	//=========== 스레드 정보 
	int ThreadMax;
	int ThreadRunCount;

	//===========  Worker TH + Accept TH
	static unsigned int WINAPI workerTh(LPVOID thisclass);
	static unsigned int WINAPI acceptTh(LPVOID thisclass);

	//=========== Worker loof + Accept loof
	void AcceptLoop();
	void WorkerLoop();

	//=========== 송수신 함수 
	void SendPost(stSession* session);
	BOOL RecvPost(stSession* session);


	//===================세션 배열 +  빈 인덱스 스택===========================//
	stSession* sessionPool;
	LockfreeStack <WORD>BlankIndexStack;
	DWORD poolCount;



	//=========== 세션 사용 함수  
	stSession* AllocSession(); 
	void ReleaseSession(stSession* session);
	void InputSession(stSession* session);
	void DeleteSession(stSession * session);
	stSession* FindSession(__int64 sessionKey);
	void CancelSession(stSession* session);


	int countalloc = 0;
	int allocfree = 0;
};

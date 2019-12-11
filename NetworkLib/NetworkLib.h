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
	};

	//session
	struct stSession
	{
		SOCKADDR_IN sockaddr;
		SOCKET		socket;

		__int64		sessionKey;

		Myoverlapped sendoverlap;
		Myoverlapped recvoverlap;

		cRingbuffer sendQ;
		cRingbuffer recvQ;

		CRITICAL_SECTION cs;

		LONG IOcount;

		LONG bSend;
		LONG bClose;

		DWORD type;

		LONG debugcount;
		char debug[100000];



	};

	virtual void onRecv(__int64 sessionKey, cMassage* msg) PURE;
	virtual void onClientJoin(__int64 sessionKey) PURE;
	virtual void onClientLeave() PURE;

private:

	//unordered_map<UINT64, SESSION*>         _MapSession;
	//typedef unordered_map<UINT64, SESSION*>   MAPSESSION;

	HANDLE hIocp;

	HANDLE* ThreadArray;

	SOCKADDR_IN serveraddr;
	SOCKET listen_socket;
	__int64 sessionNum;

	static unsigned int WINAPI workerTh(LPVOID thisclass);
	static unsigned int WINAPI acceptTh(LPVOID thisclass);

	void AcceptLoop();
	void WorkerLoop();

	int ThreadMax;
	int ThreadRunCount;

	void SendPost(stSession* session);
	BOOL RecvPost(stSession* session);

	//session pool
	CRITICAL_SECTION pool_cs;
	std::list<stSession*> sessionPool;
	
	DWORD poolCount;

	stSession* AllocSession();
	void ReleaseSession(stSession* session);

	//session map
	SRWLOCK map_cs;
	std::unordered_map<__int64, stSession*> sessionMap;


	int closecount=0;
	std::unordered_map<__int64, stSession*> closeMap;

	void InputSession(stSession* session);
	void DeleteSession(stSession * session);
	stSession* FindSession(__int64 sessionKey);
};

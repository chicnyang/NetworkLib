#pragma once

class cNetworkLib
{
public:
	cNetworkLib();
	virtual ~cNetworkLib();

	//server initail ���� ����ü
	// ���� IP Port  ���� / ��Ŀ ������ �ִ밹�� / ���� ������ ���� / ����Ǯ ���� 
	struct ServerSetting
	{
		SOCKADDR_IN sockaddr;
		int ThreadMax;
		int ThreadRun;
		int SessionPoolCount;
	};
	

	//���������� ȣ���� �Լ���
	void SendPacket(__int64 sessionKey, cMassage* packet);
	void Disconnect(__int64 sessionKey);

	//���� ���� / ���� ���� 

	void StartNetserver(ServerSetting* serversetting);
	void CloseNetserver();


	//���� ��Ȳ ����

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
		BYTE mode; //send ���� recv ���� ����
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


		LockfreeQue<cMassage*>* sendBufstack; //send �� ����ȭ���� ��� ���� 
		LONG sendCount;		//Send ����ȭ���� ī��Ʈ 

		LONG bSend;			//���� �÷��� 
		LONG bClose;		//���� �÷��� 


		stRelease refCnt;
		//LONG bRelease;		//������ �÷��� 
		//LONG IOcount;		//���� ī��Ʈ 


		DWORD type;			//��� �÷���

		DWORD startsend = 0; 

		WORD ArrayIndex;	//�ε��� ��ȣ 


	};

	virtual void onRecv(__int64 sessionKey, cMassage* msg) PURE;
	virtual void onClientJoin(__int64 sessionKey) PURE;
	virtual void onClientLeave() PURE;
	//virtual bool OnConnectionRequest(const char* clientIP,int Port) PURE;

private:


	//=========== IOCP �ڵ�
	HANDLE hIocp;

	//=========== ������ �ڵ� �迭
	HANDLE* ThreadArray;

	//=========== ���� ADDR
	SOCKADDR_IN serveraddr;

	//=========== listen ����
	SOCKET listen_socket;

	//=========== ���� �ѹ� +1
	__int64 sessionNum;

	//=========== ������ ���� 
	int ThreadMax;
	int ThreadRunCount;

	//===========  Worker TH + Accept TH
	static unsigned int WINAPI workerTh(LPVOID thisclass);
	static unsigned int WINAPI acceptTh(LPVOID thisclass);

	//=========== Worker loof + Accept loof
	void AcceptLoop();
	void WorkerLoop();

	//=========== �ۼ��� �Լ� 
	void SendPost(stSession* session);
	BOOL RecvPost(stSession* session);


	//===================���� �迭 +  �� �ε��� ����===========================//
	stSession* sessionPool;
	LockfreeStack <WORD>BlankIndexStack;
	DWORD poolCount;



	//=========== ���� ��� �Լ�  
	stSession* AllocSession(); 
	void ReleaseSession(stSession* session);
	void InputSession(stSession* session);
	void DeleteSession(stSession * session);
	stSession* FindSession(__int64 sessionKey);
	void CancelSession(stSession* session);


	int countalloc = 0;
	int allocfree = 0;
};

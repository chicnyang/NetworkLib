#pragma once

//메시지 담는함수 
//메시지 한번에 하나만담고 일회용으로 사용한다. 

class cMassage
{

public:

	static void MemoryPool(int size = 0);

	static cMassage* Alloc();

	void Free();

	BOOL bAlloc = false;

	//메시지 청소  -  버퍼 비우기 
	void Clear(void);

	//버퍼 사이즈 얻기
	int Getbufsize(void);

	//사용중인 사이즈 얻기
	int Getusesize(void);
	int GetHeaderusesize(void);

	//버퍼포인터 얻기
	BYTE* GetHeaderbufferptr(void);
	//페이로드 포인터 얻기 
	BYTE* Getbufferptr(void);

	//버퍼 ptroffset 이동.
	int MoveWritepos(int pos);
	int MoveReadpos(int pos);

	//연산자 오버로딩 넣기 빼기
	cMassage &operator = (cMassage &srcMassage);

	//넣기 
	cMassage &operator << (BYTE bValue);
	cMassage &operator << (char bValue);

	cMassage &operator << (short bValue);
	cMassage &operator << (WORD bValue);
	
	cMassage &operator << (int bValue);
	cMassage &operator << (float bValue);

	cMassage &operator << (DWORD bValue);
	cMassage &operator << (double bValue);
	cMassage &operator << (__int64 bValue);


	//빼기
	cMassage &operator >> (BYTE &bValue);
	cMassage &operator >> (char &bValue);

	cMassage &operator >> (short &bValue);
	cMassage &operator >> (WORD &bValue);

	cMassage &operator >> (int &bValue);
	cMassage &operator >> (float &bValue);

	cMassage &operator >> (DWORD &bValue);
	cMassage &operator >> (double &bValue);
	cMassage &operator >> (__int64 &bValue);

	//데이터 얻기 
	int GetData(BYTE *Dest, int iSize);

	//데이터 삽입
	int InputData(BYTE *Src, int iSize);

	//빼다가 오류 났는지 확인
	BOOL sizeerr();

	static cMemorypoolTLS<cMassage>* packetPool;

	static int GetUsePacket();
	static int GetsizePacketPool();


	void refcntUp();
	void refcntDown();

private:

	BYTE* pBuf;
	int front;
	int rear;
	BYTE* msgptr;
	BYTE* pHeader;
	int headersize;

	//헤더 세팅 함수 
	void settingHeader(int byte);
	friend class  cNetworkLib;
	friend class  cChunk<cMassage>;


	cMassage();
	//cMassage(int ibufsize);
	virtual ~cMassage() { Release(); }


	enum eMassage  //define -> 모든 소스파일에서 사용 enum 으로 클래스 함수 안에서만 사용 public 으로 밖에서 접근시 값확인만 가능 
	{
		eBuffer_defaultsize = 1000  //버퍼 기본사이즈
	};

	void resize(int size);

	//메시지 파괴  -  버퍼 지우기 
	void Release(void);

	//버퍼 사용중인 사이즈 
	int bufUsesize=0;
	//버퍼 총 사이즈 
	int bufsize;

	LONG refcount;


	//static LONG debugcount;
	//static char debugbuf[10000];

	LONG packetcount=0;
	char packetdebug[10000] = {0,};

	BOOL sendflag = false;

	int packetsize = 0;

	//사이즈 오류 플래그
	BOOL bSizenotuse = false;

};

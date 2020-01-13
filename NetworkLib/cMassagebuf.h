#pragma once

//�޽��� ����Լ� 
//�޽��� �ѹ��� �ϳ������ ��ȸ������ ����Ѵ�. 

class cMassage
{

public:

	static void MemoryPool(int size = 0);

	static cMassage* Alloc();

	void Free();

	BOOL bAlloc = false;

	//�޽��� û��  -  ���� ���� 
	void Clear(void);

	//���� ������ ���
	int Getbufsize(void);

	//������� ������ ���
	int Getusesize(void);
	int GetHeaderusesize(void);

	//���������� ���
	BYTE* GetHeaderbufferptr(void);
	//���̷ε� ������ ��� 
	BYTE* Getbufferptr(void);

	//���� ptroffset �̵�.
	int MoveWritepos(int pos);
	int MoveReadpos(int pos);

	//������ �����ε� �ֱ� ����
	cMassage &operator = (cMassage &srcMassage);

	//�ֱ� 
	cMassage &operator << (BYTE bValue);
	cMassage &operator << (char bValue);

	cMassage &operator << (short bValue);
	cMassage &operator << (WORD bValue);
	
	cMassage &operator << (int bValue);
	cMassage &operator << (float bValue);

	cMassage &operator << (DWORD bValue);
	cMassage &operator << (double bValue);
	cMassage &operator << (__int64 bValue);


	//����
	cMassage &operator >> (BYTE &bValue);
	cMassage &operator >> (char &bValue);

	cMassage &operator >> (short &bValue);
	cMassage &operator >> (WORD &bValue);

	cMassage &operator >> (int &bValue);
	cMassage &operator >> (float &bValue);

	cMassage &operator >> (DWORD &bValue);
	cMassage &operator >> (double &bValue);
	cMassage &operator >> (__int64 &bValue);

	//������ ��� 
	int GetData(BYTE *Dest, int iSize);

	//������ ����
	int InputData(BYTE *Src, int iSize);

	//���ٰ� ���� ������ Ȯ��
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

	//��� ���� �Լ� 
	void settingHeader(int byte);
	friend class  cNetworkLib;
	friend class  cChunk<cMassage>;


	cMassage();
	//cMassage(int ibufsize);
	virtual ~cMassage() { Release(); }


	enum eMassage  //define -> ��� �ҽ����Ͽ��� ��� enum ���� Ŭ���� �Լ� �ȿ����� ��� public ���� �ۿ��� ���ٽ� ��Ȯ�θ� ���� 
	{
		eBuffer_defaultsize = 1000  //���� �⺻������
	};

	void resize(int size);

	//�޽��� �ı�  -  ���� ����� 
	void Release(void);

	//���� ������� ������ 
	int bufUsesize=0;
	//���� �� ������ 
	int bufsize;

	LONG refcount;


	//static LONG debugcount;
	//static char debugbuf[10000];

	LONG packetcount=0;
	char packetdebug[10000] = {0,};

	BOOL sendflag = false;

	int packetsize = 0;

	//������ ���� �÷���
	BOOL bSizenotuse = false;

};

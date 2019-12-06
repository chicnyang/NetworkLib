#pragma once

//�޽��� ����Լ� 
//�޽��� �ѹ��� �ϳ������ ��ȸ������ ����Ѵ�. 

class cMassage
{
protected:

	BYTE* pBuf;
	int front;
	int rear;
	HANDLE hHeap;



public:
	enum eMassage  //define -> ��� �ҽ����Ͽ��� ��� enum ���� Ŭ���� �Լ� �ȿ����� ��� public ���� �ۿ��� ���ٽ� ��Ȯ�θ� ���� 
	{
		eBuffer_defaultsize = 1000  //���� �⺻������
	};
	
	cMassage();
	//cMassage(int ibufsize);

	virtual ~cMassage() { Release(); }
	void resize(int size);


	//�޽��� �ı�  -  ���� ����� 
	void Release(void);

	//�޽��� û��  -  ���� ���� 
	void Clear(void);

	//���� ������ ���
	int Getbufsize(void);

	//������� ������ ���
	int Getusesize(void);


	//���������� ���
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

protected:
	//���� ������� ������ 
	int bufUsesize=0;
	//���� �� ������ 
	int bufsize;

	//������ ���� �÷���
	BOOL bSizenotuse = false;

};

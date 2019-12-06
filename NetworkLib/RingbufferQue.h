#ifndef RINGBUFFER_QUE
#define RINGBUFFER_QUE

class cRingbuffer
{
	//char *front;
	//char *rear;//rear �� �����Ʈ 

	int front;
	int rear;

	int bufsize;//���� ������ 

	BYTE *start;
	
	SRWLOCK ringbuf_srw;

	//void Intial(int ibufsize);

public:
	enum eMassage  //define -> ��� �ҽ����Ͽ��� ��� enum ���� Ŭ���� �Լ� �ȿ����� ��� public ���� �ۿ��� ���ٽ� ��Ȯ�θ� ���� 
	{
		erBuffer_defaultsize = 10000  //���� �⺻������
	};

	cRingbuffer();
	~cRingbuffer();

	void Resize(int size);
	int Getquesize();  //���� ������� �뷮 ���
	int Getquefreesize(); //���� ��밡���� (����)�뷮 ���

//	int Getrear() { return rear; }
	//int Getfront() { return front; }

	int DirectEnquesize(); //�������ʰ� �ѹ��� ������ �ִ� �뷮
	int DirectDequesize();//�������ʰ� �ѹ��� ���� �ִ� �뷮

	int Enque(const BYTE* data, int isize);
	int Deque(BYTE* Destbuf, int isize);

	int Peek(BYTE* Destbuf, int isize);

	int Moverear(int isize);
	int Movefront(int isize);

	void Clearbuf(void);//��� ������ ����

	BYTE* Getfrontptr(void);
	BYTE* Getrearptr(void);
	BYTE* Getbufptr(void);

	//lock unlock

	void Lock();
	void Unlock();


};



#endif // !RINGBUFFER_QUE


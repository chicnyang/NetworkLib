#ifndef RINGBUFFER_QUE
#define RINGBUFFER_QUE

class cRingbuffer
{
	//char *front;
	//char *rear;//rear 가 빈바이트 

	int front;
	int rear;

	int bufsize;//버퍼 사이즈 

	BYTE *start;
	
	SRWLOCK ringbuf_srw;

	//void Intial(int ibufsize);

public:
	enum eMassage  //define -> 모든 소스파일에서 사용 enum 으로 클래스 함수 안에서만 사용 public 으로 밖에서 접근시 값확인만 가능 
	{
		erBuffer_defaultsize = 10000  //버퍼 기본사이즈
	};

	cRingbuffer();
	~cRingbuffer();

	void Resize(int size);
	int Getquesize();  //현재 사용중인 용량 얻기
	int Getquefreesize(); //현재 사용가능한 (남은)용량 얻기

//	int Getrear() { return rear; }
	//int Getfront() { return front; }

	int DirectEnquesize(); //끊기지않고 한번에 넣을수 있는 용량
	int DirectDequesize();//끊기지않고 한번에 뺄수 있는 용량

	int Enque(const BYTE* data, int isize);
	int Deque(BYTE* Destbuf, int isize);

	int Peek(BYTE* Destbuf, int isize);

	int Moverear(int isize);
	int Movefront(int isize);

	void Clearbuf(void);//모든 데이터 삭제

	BYTE* Getfrontptr(void);
	BYTE* Getrearptr(void);
	BYTE* Getbufptr(void);

	//lock unlock

	void Lock();
	void Unlock();


};



#endif // !RINGBUFFER_QUE


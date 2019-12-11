#include "pch.h"



#define SERVERPORT 6000



int main()
{
	cEchoserver server;

	SOCKADDR_IN serveraddr;
	ZeroMemory(&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(SERVERPORT);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	cEchoserver::ServerSetting serversetting;
	serversetting.sockaddr = serveraddr;
	serversetting.SessionPoolCount = 1000;
	serversetting.ThreadMax = 10;
	serversetting.ThreadRun = 0;

	cMassage::MemoryPool();

	timeBeginPeriod(1);

	server.StartNetserver(&serversetting);



	while (true)
	{


		//if (key == 'z')
		//{
		//	server.CloseNetserver();
		//	//server.~cEchoserver();
		//}
		//if (key == 'x')
		//{
		//	server.StartNetserver(&serversetting);
		//}
		//if (key == 'c')
		//{
		//	server.~cEchoserver();
		//}
		wprintf(L"\n");
		wprintf(L"//=================================================================//\n");
		wprintf(L"//=================================================================//\n\n");
		wprintf(L"Connect Session		: %d \n", server.ConnectSessioncount);
		wprintf(L"Accept Total Session    : %ld \n", server.AcceptTotalCount);
		wprintf(L"Accept TPS		: %d \n", server.AcceptCount);
		wprintf(L"Recv TPS		: %d \n", server.RecvCount);
		if (server.RecvCount < 70000)
		{

		}
		wprintf(L"Send TPS		: %d \n", server.SendCount);
		wprintf(L"Use Packet Count	: %d \n", cMassage::GetUsePacket());
		wprintf(L"Packet Pool Capacity	: %d \n", cMassage::GetsizePacketPool());
		if (server.sendcounttime != 0)
		{
			wprintf(L"send io avarge	: %f \n", ((double)server.Plustime / (double)server.sendcounttime));
		}
		if (server.sendpluscounttime != 0)
		{
			wprintf(L"send use time	: %f \n", ((double)server.sendPlustime / (double)server.sendpluscounttime));
		}
		wprintf(L"\n//=================================================================//\n");
		wprintf(L"//=================================================================//\n");
		wprintf(L"\n");
		InterlockedExchange(&server.RecvCount, 0);
		InterlockedExchange(&server.SendCount, 0);
		server.AcceptCount = 0;
		Sleep(1000);

	}





}

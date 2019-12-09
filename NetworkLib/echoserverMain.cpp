#include "pch.h"



#define SERVERPORT 6000

cDump dump;


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
	serversetting.SessionPoolCount = 3000;
	serversetting.ThreadMax = 10;
	serversetting.ThreadRun = 0;

	server.StartNetserver(&serversetting);

	while (true)
	{

		char key = _getwch();
		if (key == 'z')
		{
			server.CloseNetserver();
			break;
		}

		Sleep(10);
	}





}

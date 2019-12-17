#include "pch.h"



void cEchoserver::onRecv(__int64 sessionKey, cMassage* msg)
{

	cMassage* cmsg = cMassage::Alloc();

	__int64 data;

	*msg >> data;

	*cmsg << data;
	if (cmsg->Getusesize() < 8)
	{
		dump.Crash();
	}

	SendPacket(sessionKey, cmsg);

	cmsg->Free();
}

void cEchoserver::onClientJoin(__int64 sessionKey)
{
	cMassage* msg = cMassage::Alloc();

	__int64 data = 0x7fffffffffffffff;

	*msg << data;

	SendPacket(sessionKey, msg); 

	msg->Free();

}
void cEchoserver::onClientLeave()
{



}
#include "pch.h"



void cEchoserver::onRecv(__int64 sessionKey, cMassage* msg)
{

	cMassage* cmsg = cMassage::Alloc();

	*cmsg = *msg;

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
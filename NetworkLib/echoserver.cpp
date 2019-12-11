#include "pch.h"



void cEchoserver::onRecv(__int64 sessionKey, cMassage* msg)
{

	//cMassage* cmsg = cMassage::Alloc();

	cMassage cmsg;
	cmsg = *msg;

	SendPacket(sessionKey, &cmsg);
}

void cEchoserver::onClientJoin(__int64 sessionKey)
{
	cMassage msg;

	__int64 data = 0x7fffffffffffffff;

	msg << data;

	SendPacket(sessionKey, &msg);

}
void cEchoserver::onClientLeave()
{



}
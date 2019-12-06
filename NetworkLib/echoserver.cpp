#include "pch.h"



void cEchoserver::onRecv(__int64 sessionKey, cMassage* msg)
{
	SendPacket(sessionKey, msg);
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
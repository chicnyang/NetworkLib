#pragma once


class cEchoserver:public cNetworkLib
{
public:

	cEchoserver() {}
	virtual ~cEchoserver() {}

	virtual void onRecv(__int64 sessionKey, cMassage* msg);
	virtual void onClientJoin(__int64 sessionKey);

	virtual void onClientLeave();



};


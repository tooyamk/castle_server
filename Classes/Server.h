#pragma once

#include "NetServer.h"

class Server {
public:
	Server();
	virtual ~Server();

	void run();

	inline NetServer* __fastcall getNetServer() {
		return _net;
	}

protected:
	NetServer* _net;
};
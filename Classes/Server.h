#pragma once

#include "NetServer.h"

class Server {
public:
	Server();
	virtual ~Server();

	void run();

protected:
	NetServer* _net;
};
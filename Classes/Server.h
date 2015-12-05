#pragma once

#include "BaseUDP.h"
#include "UDPBuffer.h"

class Server {
public:
	Server();
	virtual ~Server();

	void run();

protected:
	BaseUDP* _udp;
	UDPBuffer* _buffer;
};
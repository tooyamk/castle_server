#pragma once

extern "C" {
#include "ikcp.h"
}

#include "BaseUDP.h"
#include "UDPBuffer.h"

class NetServer {
public:
	NetServer();
	virtual ~NetServer();

	void __fastcall run();
	void __fastcall __send(const char* data, unsigned int len);

protected:
	BaseUDP* _udp;
	ikcpcb* _kcp;
	UDPBuffer* _sendBuffer;
	UDPBuffer* _receiveBuffer;
	char _buffer[UDPBuffer::DataBuffer::MAX_LEN];
	sockaddr _addr;

	void __fastcall _sendHandler();
	void __fastcall _reciveHandler();
};
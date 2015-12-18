#pragma once

extern "C" {
#include "ikcp.h"
}

#include "BaseNet.h"
#include "NetDataBuffer.h"

class Client;

class NetServer {
public:
	NetServer();
	virtual ~NetServer();

	void __fastcall run();
	void __fastcall __send(const char* data, unsigned int len);
	inline NetDataBuffer* __fastcall getUDPSendBuffer() {
		return _udpSendBuffer;
	}
	inline NetDataBuffer* __fastcall getUDPReceiveBuffer() {
		return _udpReceiveBuffer;
	}

protected:
	BaseNet* _socket;
	BaseNet* _udp;
	ikcpcb* _kcp;
	NetDataBuffer* _udpSendBuffer;
	NetDataBuffer* _udpReceiveBuffer;

	void __fastcall _connectHandler();
	void __fastcall _socketAcceptHandler();
	void __fastcall _udpSendHandler();
	void __fastcall _udpReciveHandler();
};
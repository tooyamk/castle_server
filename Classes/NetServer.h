#pragma once

extern "C" {
#include "ikcp.h"
}

#include "BaseNet.h"
#include "NetDataBuffer.h"
#include <mutex>

class Client;

class NetServer {
public:
	NetServer();
	virtual ~NetServer();

	void __fastcall run();
	void __fastcall __send(const char* data, unsigned int len);
	void __fastcall sendUDP(const char* data, int len, sockaddr_in* addr);

protected:
	std::recursive_mutex _mtx;

	BaseNet* _socket;
	BaseNet* _udp;
	ikcpcb* _kcp;
	NetDataBuffer* _udpSendBuffer;
	NetDataBuffer* _udpReceiveBuffer;
	ByteArray* _udpReciveBytes;

	void __fastcall _connectHandler();
	void __fastcall _socketAcceptHandler();
	void __fastcall _udpSendHandler();
	void __fastcall _udpReciveHandler();
	void __fastcall _udpDispatchHandler();
};
#pragma once

#include "BaseUDP.h"

//#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

class UDPWin32 : public BaseUDP {
public:
	UDPWin32();
	virtual ~UDPWin32();

	virtual void __fastcall start(char* addr, int port);
	virtual void __fastcall send(const char* buf, int len, sockaddr* addr);
	virtual int __fastcall receive(char* buf, int len, sockaddr* addr);
	virtual void __fastcall close();
	virtual UDPState __fastcall getState();

protected:
	SOCKET _socket;
	sockaddr_in _serverAddr;
	int _addrLen;
	UDPState _state;
};

//#endif
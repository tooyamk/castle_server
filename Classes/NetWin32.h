#pragma once

#include "BaseNet.h"

#if (TARGET_PLATFORM == PLATFORM_WIN32)

#include <Winsock2.h>

class SocketWin32 : public BaseNet {
public:
	SocketWin32();
	virtual ~SocketWin32();

	virtual void __fastcall start(char* addr, int port);
	virtual void __fastcall start(unsigned int s);
	virtual void __fastcall sendData(const char* buf, int len);
	virtual int __fastcall receiveData(char* buf, int len);
	virtual unsigned int __fastcall acceptClient(sockaddr* addr, int* addrLen);
	virtual void __fastcall close();
	virtual ConnectState __fastcall getState();

protected:
	SOCKET _socket;
	sockaddr_in _serverAddr;
	int _addrLen;
	ConnectState _state;
};


class UDPWin32 : public BaseNet {
public:
	UDPWin32();
	virtual ~UDPWin32();

	virtual void __fastcall start(char* addr, int port);
	virtual void __fastcall sendData(const char* buf, int len);
	virtual int __fastcall receiveData(char* buf, int len);
	virtual void __fastcall close();
	virtual ConnectState __fastcall getState();

protected:
	SOCKET _socket;
	sockaddr_in _serverAddr;
	int _addrLen;
	ConnectState _state;
};

#endif
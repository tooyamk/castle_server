#pragma once

#include <Winsock2.h>

enum class UDPState {
	DISCONNECTED,
	CONNECTING,
	CONNECTED
};

class BaseUDP {
public:
	BaseUDP();
	virtual ~BaseUDP();
	virtual void __fastcall start(char* addr, int port);
	virtual void __fastcall send(const char* buf, int len, sockaddr* addr);
	virtual int __fastcall receive(char* buf, int len, sockaddr* addr);
	virtual void __fastcall close();
	virtual UDPState __fastcall getState();
};
#pragma once

struct sockaddr;

enum class NetType {
	SOCKET,
	UDP
};

enum class ConnectState {
	DISCONNECTED,
	DISCONNECTING,
	CONNECTING,
	CONNECTED
};


class BaseNet {
public:
	BaseNet();
	virtual ~BaseNet();

	virtual void __fastcall start(char* addr, int port);
	virtual void __fastcall start(unsigned int s);
	virtual void __fastcall close();
	virtual void __fastcall sendData(const char* buf, int len);
	virtual int __fastcall receiveData(char* buf, int len);
	virtual unsigned int __fastcall acceptClient(sockaddr* addr, int* addrLen);
	virtual ConnectState __fastcall getState();
};
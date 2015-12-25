#include "NetWin32.h"
#include <stdio.h>

#if (TARGET_PLATFORM == PLATFORM_WIN32)

SocketWin32::SocketWin32() {
	_socket = 0;
	_addrLen = sizeof(sockaddr_in);
	memset(&_serverAddr, 0, sizeof(_serverAddr));
	_state = ConnectState::DISCONNECTED;
}

SocketWin32::~SocketWin32() {
}

void SocketWin32::start(char* addr, int port) {
	if (_state != ConnectState::DISCONNECTED) return;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {
		printf("Winsock init faied!\r\n");
		WSACleanup();

		return;
	}
	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return;
	}

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(port);
	if (addr == nullptr) {
		_serverAddr.sin_addr.s_addr = INADDR_ANY;
	} else {
		_serverAddr.sin_addr.S_un.S_addr = inet_addr(addr);
	}

	printf("socket start ...\r\n");

	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (addr == nullptr) {
		bind(_socket, (struct sockaddr*)&_serverAddr, _addrLen);
		listen(_socket, 5);
	} else {
		if (connect(_socket, (struct sockaddr *) &_serverAddr, sizeof(_serverAddr)) < 0) {
			close();
			return;
		}
	}

	//client = accept(srvSock, (struct sockaddr *)&addrSrv, (int *)&len);

	_state = ConnectState::CONNECTED;
}

void SocketWin32::start(unsigned int s) {
	_socket = s;
	_state = ConnectState::CONNECTED;

	long o = 1;
	setsockopt(_socket, IPPROTO_TCP, TCP_NODELAY, (char*)&o, sizeof(long));
}

void SocketWin32::sendData(const char* buf, int len, sockaddr* addr) {
	if (_state != ConnectState::CONNECTED) return;
	send(_socket, buf, len, 0);
}

int SocketWin32::receiveData(char* buf, int len, sockaddr* addr) {
	if (_state != ConnectState::CONNECTED) return -1;
	//CCLOG("start recive");
	int len1 = recv(_socket, buf, len, 0);
	//if (len1 >= 0) printf("recive socket %d \n", len1);
	//CCLOG("end recive %d", len1);
	return len1;
}

unsigned int SocketWin32::acceptClient(sockaddr* addr, int* addrLen) {
	return accept(_socket, addr, addrLen);
}

ConnectState SocketWin32::getState() {
	return _state;
}

void SocketWin32::close() {
	if (_socket != 0) {
		closesocket(_socket);
		WSACleanup();
		_socket = 0;
	}

	_state = ConnectState::DISCONNECTED;
}


UDPWin32::UDPWin32() {
	_socket = 0;
	_addrLen = sizeof(sockaddr_in);
	memset(&_serverAddr, 0, sizeof(_serverAddr));
	_state = ConnectState::DISCONNECTED;
}

UDPWin32::~UDPWin32() {
	close();
}

ConnectState UDPWin32::getState() {
	return _state;
}

void UDPWin32::start(char* addr, int port) {
	if (_state != ConnectState::DISCONNECTED) return;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0) {
		printf("Winsock init faied!\r\n");
		WSACleanup();

		return;
	}

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(port);
	if (addr == nullptr) {
		_serverAddr.sin_addr.s_addr = INADDR_ANY;
	} else {
		_serverAddr.sin_addr.s_addr = inet_addr(addr);
	}

	printf("udp start ...\r\n");

	_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (addr == nullptr) {
		bind(_socket, (struct sockaddr*)&_serverAddr, sizeof(sockaddr_in));
	}

	_state = ConnectState::CONNECTED;
}

void UDPWin32::sendData(const char* buf, int len, sockaddr* addr) {
	if (_state != ConnectState::CONNECTED) return;
	sendto(_socket, buf, len, 0, addr, _addrLen);
}

int UDPWin32::receiveData(char* buf, int len, sockaddr* addr) {
	if (_state != ConnectState::CONNECTED) return -1;
	//CCLOG("start recive");
	if (addr == nullptr) addr = (sockaddr*)&_serverAddr;
	int len1 = recvfrom(_socket, buf, len, 0, addr, &_addrLen);
	//if (len1 >= 0) printf("recive udp %d \n", len1);
	//CCLOG("end recive %d", len1);
	return len1;
}

void UDPWin32::close() {
	if (_socket != 0) {
		closesocket(_socket);
		WSACleanup();
		_socket = 0;
	}

	_state = ConnectState::DISCONNECTED;
}

#endif
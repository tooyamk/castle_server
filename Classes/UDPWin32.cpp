#include "UDPWin32.h"
#include <stdio.h>

//#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

UDPWin32::UDPWin32() {
	_socket = 0;
	_addrLen = sizeof(sockaddr_in);
	_state = UDPState::DISCONNECTED;
}

UDPWin32::~UDPWin32() {
	close();
}

UDPState UDPWin32::getState() {
	return _state;
}

void UDPWin32::start(char* addr, int port) {
	if (_state != UDPState::DISCONNECTED) return;

	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0) {
		printf("Winsock init faied!\r\n");
		WSACleanup();

		return;
	}

	memset(&_serverAddr, 0, sizeof(_serverAddr));

	_serverAddr.sin_family = AF_INET;
	_serverAddr.sin_port = htons(port);
	if (addr == nullptr) {
		_serverAddr.sin_addr.s_addr = INADDR_ANY;
	} else {
		_serverAddr.sin_addr.s_addr = inet_addr(addr);
	}

	printf("Now connecting the server...\r\n");

	_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (addr == nullptr) {
		bind(_socket, (struct sockaddr*)&_serverAddr, sizeof(sockaddr_in));
	}

	_state = UDPState::CONNECTED;
}

void UDPWin32::send(const char* buf, int len, sockaddr* addr) {
	if (_state != UDPState::CONNECTED) return;
	sendto(_socket, buf, len, 0, addr, _addrLen);
}

//#include "cocos2d.h"

int UDPWin32::receive(char* buf, int len, sockaddr* addr) {
	if (_state != UDPState::CONNECTED) return -1;
	//CCLOG("start recive");
	int len1 = recvfrom(_socket, buf, len, 0, addr, &_addrLen);
	if (len1 > 0) printf("recive %d", len1);
	//CCLOG("end recive %d", len1);
	return len1;
	//return recvfrom(_socket, buf, len, 0, (struct sockaddr*)&_serverAddr, &_addrLen);
}

void UDPWin32::close() {
	closesocket(_socket);
	WSACleanup();

	_state = UDPState::DISCONNECTED;
}

//#endif
#include "NetServer.h"
#include "NetWin32.h"
#include <thread>
#include "Client.h"

int _udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
	//NetServer* c = (NetServer*)user;
	//c->__send(buf, len);
	return 0;
}

NetServer::NetServer() {
	_socket = new SocketWin32();
	_udp = new UDPWin32();

	_udpSendBuffer = new NetDataBuffer();
	_udpReceiveBuffer = new NetDataBuffer();
}

NetServer::~NetServer() {
}

void NetServer::run() {
	int prot = 6000;

	_udpSendBuffer->create();
	_udpReceiveBuffer->create();

	_socket->start(nullptr, 6000);
	_udp->start(nullptr, prot + 1);

	std::thread udpReceiveThread(&NetServer::_udpSendHandler, this);
	std::thread udpSendThread(&NetServer::_udpReciveHandler, this);
	udpReceiveThread.detach();
	udpSendThread.detach();

	std::thread socketAcceptThread(&NetServer::_socketAcceptHandler, this);
	socketAcceptThread.detach();
}

void NetServer::_socketAcceptHandler() {
	while (true) {
		sockaddr_in addr;
		int addrLen = sizeof(addr);
		memset(&addr, 0, addrLen);
		
		unsigned int s = _socket->acceptClient((struct sockaddr*)&addr, &addrLen);
		Client* c = new Client(s, addr);
		Client::addClient(c);
		c->run();

		Sleep(1);
	}
}

void NetServer::_udpSendHandler() {
	while (true) {
		while (_udpSendBuffer->send(_udp)) {
		}

		Sleep(1);
	}
}

void NetServer::_udpReciveHandler() {
	while (true) {
		_udpReceiveBuffer->receive(_udp);
		_udpReceiveBuffer->read(nullptr, 0);

		Sleep(1);
	}
}
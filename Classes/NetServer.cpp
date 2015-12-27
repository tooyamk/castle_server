#include "NetServer.h"
#include "NetWin32.h"
#include <thread>
#include "Client.h"
#include "ByteArray.h"
#include "Packet.h"

NetServer::NetServer() {
	_socket = new SocketWin32();
	_udp = new UDPWin32();

	_udpSendBuffer = new NetDataBuffer();
	_udpReceiveBuffer = new NetDataBuffer();
	_udpReciveBytes = new ByteArray(false);
}

NetServer::~NetServer() {
}

void NetServer::run() {
	int prot = 6000;

	_udpSendBuffer->create();
	_udpReceiveBuffer->create();

	_socket->start(nullptr, 6000);
	_udp->start(nullptr, prot + 1);

	std::thread udpDispatchThread(&NetServer::_udpDispatchHandler, this);
	std::thread udpReceiveThread(&NetServer::_udpSendHandler, this);
	std::thread udpSendThread(&NetServer::_udpReciveHandler, this);
	udpDispatchThread.detach();
	udpReceiveThread.detach();
	udpSendThread.detach();

	std::thread socketAcceptThread(&NetServer::_socketAcceptHandler, this);
	socketAcceptThread.detach();
}

void NetServer::sendUDP(bool kcp, const char* data, int len, sockaddr_in* addr) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	_udpSendBuffer->write(kcp, data, len, addr);
}

void NetServer::_socketAcceptHandler() {
	while (true) {
		sockaddr_in addr;
		int addrLen = sizeof(addr);
		memset(&addr, 0, addrLen);
		
		unsigned int s = _socket->acceptClient((struct sockaddr*)&addr, &addrLen);
		Client* c = new Client(s, addr, this);
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

		Sleep(1);
	}
}

void NetServer::_udpDispatchHandler() {
	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));

	while (true) {
		while (_udpReceiveBuffer->read(_udpReciveBytes, (sockaddr*)&addr)) {
			unsigned int readSize = 0;
			_udpReciveBytes->setPosition(0);
			while (_udpReciveBytes->getBytesAvailable() > 1) {
				unsigned short size = _udpReciveBytes->readUnsignedShort();
				if (_udpReciveBytes->getBytesAvailable() >= size) {
					readSize = size + 2;
					unsigned int id = _udpReciveBytes->readUnsignedInt();
					bool kcp = _udpReciveBytes->readBool();
					std::tr1::shared_ptr<Client> c = Client::getClient(id);
					if (c != nullptr) {
						c->receiveUDP(_udpReciveBytes, kcp, size - 5, &addr);
					}

					_udpReciveBytes->setPosition(readSize);
				} else {
					break;
				}
			}

			if (readSize > 0) _udpReciveBytes->popFront(readSize);
			_udpReciveBytes->setPosition(_udpReciveBytes->getLength());
			
		}

		Sleep(1);
	}
}
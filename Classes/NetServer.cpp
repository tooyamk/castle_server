#include "NetServer.h"
#include "UDPWin32.h"
#include <thread>

int _udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
	NetServer* c = (NetServer*)user;
	c->__send(buf, len);
	return 0;
}

NetServer::NetServer() {
	_udp = new UDPWin32();
	_sendBuffer->create();
	_receiveBuffer->create();
}

NetServer::~NetServer() {
}

void NetServer::run() {
	_udp->start(nullptr, 6000);
	_kcp = ikcp_create(0xFFFFFFFF, this);
	ikcp_nodelay(_kcp, 1, 10, 2, 1);
	ikcp_nodelay(_kcp, 1, 10, 2, 1);
	_kcp->output = _udp_output;

	std::thread receiveThread(&NetServer::_reciveHandler, this);
	std::thread sendThread(&NetServer::_sendHandler, this);
	receiveThread.detach();
	sendThread.detach();

	while (true) {
		while (_receiveBuffer->read(_buffer, UDPBuffer::DataBuffer::MAX_LEN, &_addr)) {
			ikcp_input(_kcp, _buffer, UDPBuffer::DataBuffer::MAX_LEN);
			int size = ikcp_recv(_kcp, _buffer, UDPBuffer::DataBuffer::MAX_LEN);
			if (size >= 0) {

			}
		}
		//if (_buffer->receive(_udp) > 0) {
		//	_buffer->send(_udp);
		//}

		Sleep(1);
	}
}

void NetServer::_sendHandler() {
	while (true) {
		while (_sendBuffer->send(_udp)) {
		}

		Sleep(1);
	}
}

void NetServer::_reciveHandler() {
	while (true) {
		_receiveBuffer->receive(_udp);

		Sleep(1);
	}
}

void NetServer::__send(const char* data, unsigned int len) {
	if (_udp->getState() == UDPState::CONNECTED) {
		_sendBuffer->write(data, len);
	}
}
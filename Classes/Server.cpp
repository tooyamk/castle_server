#include "Server.h"
#include "UDPWin32.h"

Server::Server() {
	_udp = new UDPWin32();
	_buffer = new UDPBuffer();
}

Server::~Server() {
}

void Server::run() {
	_udp->start(nullptr, 6000);

	while (true) {
		if (_buffer->receive(_udp) > 0) {
			_buffer->send(_udp);
		}
	}
}
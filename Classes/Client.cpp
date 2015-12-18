#include "Client.h"
#include "NetDataBuffer.h"
#include "ByteArray.h"
#include <thread>
#include "Packet.h"
#include <Ws2tcpip.h>
#include "Room.h"

Client::Client(unsigned int socket, const sockaddr_in& addr) {
	_id = _idAccumulator++;
	_addr = addr;

	curRoom = nullptr;

	_socket = new SocketWin32();
	_socket->start(socket);

	_socketReceiveBuffer = new NetDataBuffer();
	_socketReceiveBuffer->create();
	_socketReciveBytes = new ByteArray(false);
}

Client::~Client() {
	delete _socket;
	delete _socketReceiveBuffer;
	delete _socketReciveBytes;

	if (curRoom != nullptr) {
		curRoom->removeClient(_id);
		curRoom = nullptr;
	}
}

unsigned int Client::_idAccumulator = 1;
std::unordered_map<unsigned int, Client*> Client::_clients = std::unordered_map<unsigned int, Client*>();

void Client::addClient(Client* c) {
	char addr_p[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &c->_addr.sin_addr, addr_p, sizeof(addr_p));
	printf("new client connected -> %s:%d\n", addr_p, ntohs(c->_addr.sin_port));

	_clients.insert(std::pair<unsigned int, Client*>(c->getID(), c));
}

void Client::remvoeClient(unsigned int id) {
	auto& itr = _clients.find(id);
	if (itr != _clients.end()) {
		Client* c = itr->second;

		char addr_p[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &c->_addr.sin_addr, addr_p, sizeof(addr_p));
		printf("client deconnected -> %s:%d\n", addr_p, ntohs(c->_addr.sin_port));

		delete c;
		_clients.erase(itr);
	}
}

void Client::run() {
	ByteArray ba(false);
	ba.writeUnsignedShort(0);
	ba.writeUnsignedShort(0x0001);
	ba.writeUnsignedShort(6001);
	ba.writeUnsignedInt(_id);
	ba.setPosition(0);
	ba.writeUnsignedShort(ba.getLength() - 2);
	_socket->sendData((const char*)ba.getBytes(), ba.getLength());

	std::thread t(&Client::_socketReceiveHandler, this);
	t.detach();
}

void Client::_socketReceiveHandler() {
	while (true) {
		if (_socketReceiveBuffer->receive(_socket) >= 0) {
			_socketReceiveBuffer->read(_socketReciveBytes);

			_socketReciveBytes->setPosition(0);
			Packet p;
			unsigned int size = Packet::parse(_socketReciveBytes, &p);
			if (size > 0) _socketReciveBytes->popFront(size);
			_socketReciveBytes->setPosition(_socketReciveBytes->getLength());

			if (p.head == 0x0100) {
				_receive0x0100(&p);
			} else if (p.head == 0x0101) {
				_receive0x0101(&p);
			}
		} else {
			break;
		}

		Sleep(1);
	}

	Client::remvoeClient(_id);
}

void Client::_receive0x0100(Packet* p) {
	if (curRoom == nullptr) {
		curRoom = Room::create();
		curRoom->addClient(_id);
	}

	ByteArray ba(false);
	ba.writeUnsignedShort(0);
	ba.writeUnsignedShort(0x0100);
	ba.writeUnsignedInt(curRoom->getID());
	ba.setPosition(0);
	ba.writeUnsignedShort(ba.getLength() - 2);
	_socket->sendData((const char*)ba.getBytes(), ba.getLength());
}

void Client::_receive0x0101(Packet* p) {
	if (curRoom != nullptr) {
		curRoom->removeClient(_id);
		curRoom = nullptr;
	}

	ByteArray ba(false);
	ba.writeUnsignedShort(0);
	ba.writeUnsignedShort(0x0101);
	ba.setPosition(0);
	ba.writeUnsignedShort(ba.getLength() - 2);
	_socket->sendData((const char*)ba.getBytes(), ba.getLength());
}
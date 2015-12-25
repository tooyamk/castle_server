#include "Client.h"
#include "NetDataBuffer.h"
#include "ByteArray.h"
#include <thread>
#include "Packet.h"
#include <Ws2tcpip.h>
#include "Room.h"
#include "NetServer.h"

Client::Client(unsigned int socket, const sockaddr_in& addr, NetServer* server) {
	_id = _idAccumulator++;
	_addr = addr;
	_server = server;

	udpAddr = nullptr;
	_isClosed = false;
	order = -1;
	ready = false;
	levelInited = 0;

	_socket = new SocketWin32();
	_socket->start(socket);

	_socketReceiveBuffer = new NetDataBuffer();
	_socketReceiveBuffer->create();
	_socketReciveBytes = new ByteArray(false);

	_self = std::tr1::shared_ptr<Client>(this);

	char addr_p[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &_addr.sin_addr, addr_p, sizeof(addr_p));
	printf("new client connected -> %s:%d\n", addr_p, ntohs(_addr.sin_port));
}

Client::~Client() {
	delete _socket;
	delete _socketReceiveBuffer;
	delete _socketReciveBytes;

	char addr_p[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &_addr.sin_addr, addr_p, sizeof(addr_p));
	printf("client deconnected -> %s:%d\n", addr_p, ntohs(_addr.sin_port));
}

std::recursive_mutex Client::_staticMtx = std::recursive_mutex();
unsigned int Client::_idAccumulator = 1;
std::unordered_map<unsigned int, Client*> Client::_clients = std::unordered_map<unsigned int, Client*>();

void Client::addClient(Client* c) {
	std::lock_guard<std::recursive_mutex> lck(_staticMtx);

	_clients.insert(std::pair<unsigned int, Client*>(c->getID(), c));
}

void Client::remvoeClient(unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(_staticMtx);

	auto& itr = _clients.find(id);
	if (itr != _clients.end()) {
		Client* c = itr->second;

		c->close();
		_clients.erase(itr);
	}
}

std::tr1::shared_ptr<Client> Client::getClient(unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(_staticMtx);

	auto& itr = _clients.find(id);
	if (itr == _clients.end()) {
		return nullptr;
	} else {
		return itr->second->getSharedPtr();
	}
}

void Client::close() {
	bool b = _mtx.try_lock();
	if (b) {
		std::lock_guard<std::recursive_mutex> lck(_mtx, std::adopt_lock);

		if (!_isClosed) {
			_isClosed = true;

			exitRoom();
		}
	}

	_self.reset();
}

void Client::receiveUDP(Packet* p, sockaddr_in* addr) {
	if (udpAddr == nullptr) {
		udpAddr = new sockaddr_in();
		memset(udpAddr, 0, sizeof(sockaddr_in));

		udpAddr->sin_addr = addr->sin_addr;
		udpAddr->sin_family = addr->sin_family;
		udpAddr->sin_port = addr->sin_port;
	}

	_executePacket(p);
}

void Client::exitRoom() {
	if (_curRoom.get() != nullptr) {
		_curRoom->removeClient(this);

		if (_curRoom->getNumClients() == 0) {
			_curRoom->close();
		}

		_curRoom.reset();
	}
}

void Client::switchReadyState() {
	if (_curRoom.get() != nullptr) {
		_curRoom->setClientReady(this, !ready);
	}
}

void Client::startLevel() {
	if (_curRoom.get() != nullptr) {
		_curRoom->startLevel(this);
	}
}

void Client::syncClient(ByteArray* ba) {
	if (_curRoom.get() != nullptr) {
		_curRoom->syncClient(this, ba);
	}
}

void Client::syncEntity(ByteArray* ba) {
	if (_curRoom.get() != nullptr) {
		_curRoom->syncEntity(this, ba);
	}
}

void Client::initLevelComplete() {
	if (_curRoom.get() != nullptr) {
		_curRoom->initLevelComplete(this);
	}
}

Room* Client::getCurRoom() {
	return _curRoom.get();
}

void Client::setCurRoom(Room* room) {
	if (room == nullptr) {
		_curRoom.reset();
	} else {
		_curRoom = room->getSharedPtr();
	}
}

void Client::sendData(const char* bytes, int len, bool udp) {
	std::lock_guard<std::recursive_mutex> lck(_sendMtx);
	
	if (udp) {
		if (udpAddr != nullptr) {
			_server->sendUDP(bytes, len, udpAddr);
		}
	} else {
		_socket->sendData(bytes, len, nullptr);
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
	_socket->sendData((const char*)ba.getBytes(), ba.getLength(), nullptr);

	std::thread t(&Client::_socketReceiveHandler, this);
	t.detach();
}

void Client::_socketReceiveHandler() {
	while (true) {
		if (_socketReceiveBuffer->receive(_socket) >= 0) {
			_socketReceiveBuffer->read(_socketReciveBytes, nullptr);

			while (_socketReciveBytes->getLength() > 0) {
				_socketReciveBytes->setPosition(0);
				Packet p;
				unsigned int size = Packet::parse(_socketReciveBytes, &p);
				if (size > 0) _socketReciveBytes->popFront(size);
				_socketReciveBytes->setPosition(_socketReciveBytes->getLength());

				if (size == 0) break;

				_executePacket(&p);
			}
		} else {
			break;
		}

		Sleep(1);
	}

	Client::remvoeClient(_id);
}

void Client::_executePacket(Packet* p) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	if (p->head == 0x0002) {
		ByteArray ba(false);
		ba.writeUnsignedShort(0);
		ba.writeUnsignedShort(0x0002);
		ba.setPosition(0);
		ba.writeUnsignedShort(ba.getLength() - 2);
		sendData((const char*)ba.getBytes(), ba.getLength(), true);
	} else if (p->head == 0x0100) {
		p->bytes.setPosition(0);
		unsigned int id = p->bytes.readUnsignedInt();
		Room::joinRoom(this, id);
	} else if (p->head == 0x0101) {
		exitRoom();
	} else if (p->head == 0x0102) {
		switchReadyState();
	} else if (p->head == 0x0103) {
		startLevel();
	} else if (p->head == 0x0200) {
		syncClient(&p->bytes);
	} else if (p->head == 0x0201) {
		initLevelComplete();
	} else if (p->head == 0x0202) {
		syncEntity(&p->bytes);
	}
}
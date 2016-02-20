#include "Client.h"
#include "NetDataBuffer.h"
#include "ByteArray.h"
#include <thread>
#include "Packet.h"
#include <Ws2tcpip.h>
#include "Room.h"
#include "NetServer.h"

static inline void itimeofday(long *sec, long *usec) {
#if defined(__unix)
	struct timeval time;
	gettimeofday(&time, NULL);
	if (sec) *sec = time.tv_sec;
	if (usec) *usec = time.tv_usec;
#else
	static long mode = 0, addsec = 0;
	BOOL retval;
	static IINT64 freq = 1;
	IINT64 qpc;
	if (mode == 0) {
		retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
		freq = (freq == 0) ? 1 : freq;
		retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
		addsec = (long)time(NULL);
		addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
		mode = 1;
	}
	retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
	retval = retval * 2;
	if (sec) *sec = (long)(qpc / freq) + addsec;
	if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
#endif
}

static inline IINT64 iclock64(void) {
	long s, u;
	IINT64 value;
	itimeofday(&s, &u);
	value = ((IINT64)s) * 1000 + (u / 1000);
	return value;
}

static inline IUINT32 iclock() {
	return (IUINT32)(iclock64() & 0xfffffffful);
}

int _udp_output(const char *buf, int len, ikcpcb *kcp, void *user) {
	Client* c = (Client*)user;
	c->__sendUDP(buf, len);
	return 0;
}

Client::Client(unsigned int socket, const sockaddr_in& addr, NetServer* server) {
	_id = _idAccumulator++;
	_addr = addr;
	_server = server;

	udpAddr = nullptr;
	_isClosed = false;
	order = -1;
	ready = false;
	levelInited = 0;

	_tcp = new SocketWin32();
	_tcp->start(socket);

	_tcpReceiveBuffer = new NetDataBuffer();
	_tcpReceiveBuffer->create();
	_kcpSendBuffer = new NetDataBuffer();
	_kcpSendBuffer->create();
	_kcpReceiveBuffer2 = new NetDataBuffer();
	_kcpReceiveBuffer2->create();
	_tcpReciveBytes = new ByteArray(false);

	_nextUpdateKCPTime = 0;
	_kcp = ikcp_create(0xFFFFFFFF, this);
	ikcp_setmtu(_kcp, 1000);
	ikcp_nodelay(_kcp, 1, 10, 2, 1);
	_kcp->output = _udp_output;

	_self = std::tr1::shared_ptr<Client>(this);

	char addr_p[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &_addr.sin_addr, addr_p, sizeof(addr_p));
	printf("new client connected -> %s:%d\n", addr_p, ntohs(_addr.sin_port));
}

Client::~Client() {
	exitRoom();

	delete _tcp;
	delete _tcpReceiveBuffer;
	delete _tcpReciveBytes;
	ikcp_release(_kcp);

	char addr_p[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &_addr.sin_addr, addr_p, sizeof(addr_p));
	printf("client deconnected -> %s:%d\n", addr_p, ntohs(_addr.sin_port));
}

std::recursive_mutex* Client::_staticMtx = new std::recursive_mutex();
unsigned int Client::_idAccumulator = 1;
std::unordered_map<unsigned int, Client*> Client::_clients = std::unordered_map<unsigned int, Client*>();

void Client::addClient(Client* c) {
	std::lock_guard<std::recursive_mutex> lck(*_staticMtx);

	_clients.insert(std::pair<unsigned int, Client*>(c->getID(), c));
}

void Client::remvoeClient(unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(*_staticMtx);

	auto& itr = _clients.find(id);
	if (itr != _clients.end()) {
		Client* c = itr->second;

		c->close();
		_clients.erase(itr);
	}
}

std::tr1::shared_ptr<Client> Client::getClient(unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(*_staticMtx);

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

void Client::receiveUDP(ByteArray* bytes, bool kcp, unsigned short len, sockaddr_in* addr) {
	if (udpAddr == nullptr) {
		udpAddr = new sockaddr_in();
		memset(udpAddr, 0, sizeof(sockaddr_in));

		udpAddr->sin_addr = addr->sin_addr;
		udpAddr->sin_family = addr->sin_family;
		udpAddr->sin_port = addr->sin_port;
	}

	if (kcp) {
		//bytes->readBytes(_udpReceiveToKcpBuffer, 0, len);
		_kcpReceiveBuffer2->write(bytes, len, nullptr);
		//ikcp_input(_kcp, _udpReceiveToKcpBuffer, len);
	} else {
		Packet p;
		p.head = bytes->readUnsignedShort();
		len -= 2;
		if (len > 0) bytes->readBytes(&p.bytes, 0, len);
		p.bytes.setPosition(0);
		_executePacket(&p);
	}
}

void Client::exitRoom() {
	if (_curRoom.get() != nullptr) {
		_curRoom->removeClient(this);
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

void Client::sendData(const char* bytes, int len, NetType type) {
	std::lock_guard<std::recursive_mutex> lck(_sendMtx);
	
	if (type == NetType::TCP) {
		char* p = (char*)&len;
		_tcpSendBuffer[0] = p[0];
		_tcpSendBuffer[1] = p[1];
		for (unsigned short i = 0; i < len; i++) {
			_tcpSendBuffer[2 + i] = bytes[i];
		}
		
		_tcp->sendData(_tcpSendBuffer, len + 2, nullptr);
	} else if (type == NetType::KCP) {
		_kcpSendBuffer->write(bytes, len, nullptr);
		//ikcp_send(_kcp, bytes, len);
	} else if (type == NetType::UDP) {
		if (udpAddr != nullptr) {
			_server->sendUDP(false, bytes, len, udpAddr);
		}
	}
}

void Client::__sendUDP(const char* data, unsigned int len) {
	if (udpAddr != nullptr) {
		_server->sendUDP(true, data, len, udpAddr);
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
	_tcp->sendData((const char*)ba.getBytes(), ba.getLength(), nullptr);

	_selfTcp = _self;
	_selfKcp = _self;

	std::thread kcpThread(&Client::_kcpHandler, this);
	kcpThread.detach();

	std::thread t(&Client::_tcpReceiveHandler, this);
	t.detach();
}

void Client::_tcpReceiveHandler() {
	while (true) {
		if (_tcpReceiveBuffer->receive(_tcp) > 0) {
			_tcpReceiveBuffer->read(_tcpReciveBytes, nullptr);

			while (_tcpReciveBytes->getLength() > 0) {
				_tcpReciveBytes->setPosition(0);
				Packet p;
				unsigned int size = Packet::parse(_tcpReciveBytes, &p);
				if (size > 0) _tcpReciveBytes->popFront(size);
				_tcpReciveBytes->setPosition(_tcpReciveBytes->getLength());

				if (size == 0) break;

				_executePacket(&p);
			}
		} else {
			break;
		}

		Sleep(1);
	}

	Client::remvoeClient(_id);

	_selfTcp.reset();
}

void Client::_kcpHandler() {
	while (true) {
		if (_isClosed) {
			break;
		} else {
			bool isChanged = false;
			while (_kcpSendBuffer->send(_kcp)) {
				isChanged = true;
			}

			while (_kcpReceiveBuffer2->read(_kcp)) {
				isChanged = true;
			}

			IUINT32 t = iclock();
			if (isChanged || t >= _nextUpdateKCPTime) {
				ikcp_update(_kcp, t);
				_nextUpdateKCPTime = ikcp_check(_kcp, t);
			}
			
			while (true) {
				int kcpSize = ikcp_recv(_kcp, _kcpReceiveBuffer, NetDataBuffer::BufferNode::MAX_LEN);
				if (kcpSize > 0) {
					ByteArray ba(false, _kcpReceiveBuffer, kcpSize);
					Packet p;
					p.head = ba.readUnsignedShort();
					ba.readBytes(&p.bytes, 0, 0);
					p.bytes.setPosition(0);
					_executePacket(&p);
				} else {
					break;
				}
			}
		}

		Sleep(1);
	}

	_selfKcp.reset();
}

void Client::_executePacket(Packet* p) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	switch (p->head) {
	case 0x0002: {
		ByteArray ba(false);
		ba.writeUnsignedShort(0x0002);
		sendData((const char*)ba.getBytes(), ba.getLength(), NetType::KCP);
		break;
	}
	case 0x0100: {
		p->bytes.setPosition(0);
		unsigned int id = p->bytes.readUnsignedInt();
		Room::joinRoom(this, id);
		break;
	}
	case 0x0101: {
		exitRoom();
		break;
	}
	case 0x0102: {
		switchReadyState();
		break;
	}
	case 0x0103: {
		startLevel();
		break;
	}
	case 0x0104: {
		Room::matchRoom(this);
		break;
	}
	case 0x0105: {
		if (_curRoom.get() != nullptr) {
			_curRoom->setGobackReadyRoom(this);
		}
		break;
	}
	case 0x0200: {
		syncClient(&p->bytes);
		break;
	}
	case 0x0201: {
		initLevelComplete();
		break;
	}
	case 0x0202: {
		syncEntity(&p->bytes);
		break;
	}
	case 0x0203: {
		if (_curRoom.get() != nullptr) {
			_curRoom->syncEntityHP(this, &p->bytes);
		}
		break;
	}
	case 0x0204: {
		if (_curRoom.get() != nullptr) {
			_curRoom->syncEntityGeneratorCreate(this, &p->bytes);
		}
		break;
	}
	case 0x0205: {
		if (_curRoom.get() != nullptr) {
			_curRoom->setBattleFinish(this, &p->bytes);
		}
		break;
	}
	default:
		break;
	}
}
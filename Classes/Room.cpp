#include "Room.h"
#include "Client.h"
#include "ByteArray.h"

Room::Room() {
	_id = _idAccumulator++;

	_host = nullptr;
	_isClosed = false;
	_clientsMask = 0;

	_battleState = BattleState::NONE;

	_self = std::tr1::shared_ptr<Room>(this);

	printf("create room -> %d\n", _id);
}

Room::~Room() {
	printf("release room -> %d\n", _id);
}

unsigned int Room::_idAccumulator = 1;
std::recursive_mutex Room::_staticMtx = std::recursive_mutex();
std::unordered_map<unsigned int, Room*> Room::_rooms = std::unordered_map<unsigned int, Room*>();

Room* Room::create() {
	std::unique_lock<std::recursive_mutex> lck(_staticMtx);
	
	Room* r = new Room();
	_rooms.insert(std::pair<unsigned int, Room*>(r->getID(), r));
	return r;
}

Room* Room::get(unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(_staticMtx);

	auto& itr = _rooms.find(id);
	return itr == _rooms.end() ? nullptr : itr->second;
}

void Room::remove(unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(_staticMtx);

	auto& itr = _rooms.find(id);
	if (itr != _rooms.end()) {
		_rooms.erase(itr);
	}
}

void Room::joinRoom(Client* c, unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(_staticMtx);

	std::string error = "";

	if (id == 0) {
		Room* room = create();
		id = room->getID();
	}

	Room* room = get(id);
	if (room == nullptr) {
		error = "no room";
	} else {
		error = room->addClient(c);
	}

	if (error.size() > 0) {
		ByteArray ba(false);
		ba.writeUnsignedShort(0);
		ba.writeUnsignedShort(0x0100);
		ba.writeUnsignedChar(0);
		ba.writeString(error.c_str());
		ba.setPosition(0);
		ba.writeUnsignedShort(ba.getLength() - 2);
		c->sendData((const char*)ba.getBytes(), ba.getLength());
	}
}

std::string Room::addClient(Client* c) {
	if (!_isClosed) {
		std::lock_guard<std::recursive_mutex> lck(_mtx);

		std::string error = "";

		if (_battleState != BattleState::NONE) {
			error = "battling";
		}

		if (c->getCurRoom() != nullptr) {
			if (c->getCurRoom()->getID() == _id) {
				error = "already in this room";
			} else if (_clients.size() >= MAX_NUM_PLAYERS) {
				error = "already full";
			} else {
				error = "already in other room";
			}
		}

		if (error.size() > 0) {
			return error;
		}

		auto& itr = _clients.find(c->getID());
		if (itr == _clients.end()) {
			c->setCurRoom(this);
			c->order = _getEmptyClientOrder();
			_setClientOrderMask(c->order, true);

			for (auto& itr : _clients) {
				Client* other = itr.second.get();

				ByteArray ba(false);
				ba.writeUnsignedShort(0);
				ba.writeUnsignedShort(0x0100);
				ba.writeUnsignedChar(3);
				ba.writeUnsignedInt(c->getID());
				ba.writeUnsignedChar(c->order);
				ba.setPosition(0);
				ba.writeUnsignedShort(ba.getLength() - 2);
				other->sendData((const char*)ba.getBytes(), ba.getLength());
			}

			_clients.insert(std::pair<unsigned int, std::tr1::shared_ptr<Client>>(c->getID(), c->getSharedPtr()));
			if (_host == nullptr) {
				_host = c;
			}

			ByteArray ba(false);
			ba.writeUnsignedShort(0);
			ba.writeUnsignedShort(0x0100);
			ba.writeUnsignedChar(1);
			ba.writeUnsignedInt(_id);
			ba.writeUnsignedInt(_host->getID());
			ba.writeUnsignedChar(_clients.size());
			for (auto& itr : _clients) {
				Client* other = itr.second.get();
				ba.writeUnsignedInt(other->getID());
				ba.writeUnsignedChar(other->order);
				ba.writeBool(other->ready);
			}
			ba.setPosition(0);
			ba.writeUnsignedShort(ba.getLength() - 2);
			c->sendData((const char*)ba.getBytes(), ba.getLength());
		}

		return "";
	}
}

void Room::removeClient(Client* c) {
	if (!_isClosed) {
		std::lock_guard<std::recursive_mutex> lck(_mtx);

		auto& itr = _clients.find(c->getID());
		if (itr != _clients.end()) {
			_clients.erase(itr);
			_setClientOrderMask(c->order, false);
			c->order = -1;
			c->ready = false;

			if (_host == c) {
				_host = nullptr;

				for (auto& itr : _clients) {
					_host = itr.second.get();
					break;
				}

				for (auto& itr : _clients) {
					Client* other = itr.second.get();

					ByteArray ba(false);
					ba.writeUnsignedShort(0);
					ba.writeUnsignedShort(0x0100);
					ba.writeUnsignedChar(5);
					ba.writeUnsignedInt(_host->getID());
					ba.setPosition(0);
					ba.writeUnsignedShort(ba.getLength() - 2);
					other->sendData((const char*)ba.getBytes(), ba.getLength());
				}
			}

			for (auto& itr : _clients) {
				Client* other = itr.second.get();

				ByteArray ba(false);
				ba.writeUnsignedShort(0);
				ba.writeUnsignedShort(0x0100);
				ba.writeUnsignedChar(4);
				ba.writeUnsignedInt(c->getID());
				ba.setPosition(0);
				ba.writeUnsignedShort(ba.getLength() - 2);
				other->sendData((const char*)ba.getBytes(), ba.getLength());
			}

			ByteArray ba(false);
			ba.writeUnsignedShort(0);
			ba.writeUnsignedShort(0x0100);
			ba.writeUnsignedChar(2);
			ba.setPosition(0);
			ba.writeUnsignedShort(ba.getLength() - 2);
			c->sendData((const char*)ba.getBytes(), ba.getLength());

			_sendLevelSyncComplete();
		}
	}
}

void Room::setClientReady(Client* c, bool b) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	if (_battleState == BattleState::NONE) {
		if (c->ready != b && c->getCurRoom() == this) {
			c->ready = b;
		}

		for (auto& itr : _clients) {
			Client* c2 = itr.second.get();

			ByteArray ba(false);
			ba.writeUnsignedShort(0);
			ba.writeUnsignedShort(0x0100);
			ba.writeUnsignedChar(6);
			ba.writeUnsignedInt(c->getID());
			ba.writeBool(c->ready);
			ba.setPosition(0);
			ba.writeUnsignedShort(ba.getLength() - 2);
			c2->sendData((const char*)ba.getBytes(), ba.getLength());
		}
	}
}

void Room::startLevel(Client* c) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	if (_battleState == BattleState::NONE) {
		bool ok = false;
		if (_host == c) {
			int n = _clients.size();
			for (auto& itr : _clients) {
				Client* c2 = itr.second.get();
				if (c2->ready) {
					n--;
				}
			}
			ok = n == 0;
		}

		if (ok) {
			for (auto& itr : _clients) {
				Client* c2 = itr.second.get();
				c2->levelInited = false;

				ByteArray ba(false);
				ba.writeUnsignedShort(0);
				ba.writeUnsignedShort(0x0100);
				ba.writeUnsignedChar(7);
				ba.writeBool(true);
				ba.setPosition(0);
				ba.writeUnsignedShort(ba.getLength() - 2);
				c2->sendData((const char*)ba.getBytes(), ba.getLength());
			}

			_syncClients = 0;
			_battleState = BattleState::INIT;
		} else {
			ByteArray ba(false);
			ba.writeUnsignedShort(0);
			ba.writeUnsignedShort(0x0100);
			ba.writeUnsignedChar(7);
			ba.writeBool(false);
			ba.setPosition(0);
			ba.writeUnsignedShort(ba.getLength() - 2);
			c->sendData((const char*)ba.getBytes(), ba.getLength());
		}
	}
}

void Room::syncClient(Client* c, ByteArray* data) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	if (_battleState == BattleState::INIT) {
		if (!c->levelInited) {
			c->levelInited = true;
			_syncClients++;

			for (auto& itr : _clients) {
				Client* c2 = itr.second.get();
				if (c2 == c) continue;

				ByteArray ba(false);
				ba.writeUnsignedShort(0);
				ba.writeUnsignedShort(0x0200);
				ba.writeUnsignedChar(0);
				ba.writeBytes(data);
				ba.setPosition(0);
				ba.writeUnsignedShort(ba.getLength() - 2);
				c2->sendData((const char*)ba.getBytes(), ba.getLength());
			}

			_sendLevelSyncComplete();
		}
	}
}

void Room::_sendLevelSyncComplete() {
	if (_battleState == BattleState::INIT && _syncClients >= _clients.size()) {
		_battleState = BattleState::RUNNING;
		for (auto& itr : _clients) {
			Client* c = itr.second.get();

			ByteArray ba(false);
			ba.writeUnsignedShort(0);
			ba.writeUnsignedShort(0x0200);
			ba.writeUnsignedChar(1);
			ba.writeUnsignedInt(_host->getID());
			ba.setPosition(0);
			ba.writeUnsignedShort(ba.getLength() - 2);
			c->sendData((const char*)ba.getBytes(), ba.getLength());
		}
	}
}

unsigned int Room::getNumClients() {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	return _clients.size();
}

void Room::close() {
	bool b = _mtx.try_lock();
	if (b) {
		std::lock_guard<std::recursive_mutex> lck(_mtx, std::adopt_lock);

		if (!_isClosed) {
			_isClosed = true;

			remove(_id);
		}
	}

	_self = nullptr;
}

char Room::_getEmptyClientOrder() {
	for (char i = 0; i < MAX_NUM_PLAYERS; i++) {
		if ((_clientsMask & (1 << i)) == 0) {
			return i;
		}
	}

	return -1;
}

void Room::_setClientOrderMask(unsigned char index, bool b) {
	if (b) {
		_clientsMask |= 1 << index;
	} else {
		_clientsMask &= ~(1 << index);
	}
}
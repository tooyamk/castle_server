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
std::recursive_mutex* Room::_staticMtx = new std::recursive_mutex();
std::unordered_map<unsigned int, Room*> Room::_rooms = std::unordered_map<unsigned int, Room*>();

Room* Room::create() {
	std::unique_lock<std::recursive_mutex> lck(*_staticMtx);
	
	Room* r = new Room();
	_rooms.insert(std::pair<unsigned int, Room*>(r->getID(), r));
	return r;
}

Room* Room::get(unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(*_staticMtx);

	auto& itr = _rooms.find(id);
	return itr == _rooms.end() ? nullptr : itr->second;
}

void Room::remove(unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(*_staticMtx);

	auto& itr = _rooms.find(id);
	if (itr != _rooms.end()) {
		_rooms.erase(itr);
	}
}

void Room::joinRoom(Client* c, unsigned int id) {
	std::lock_guard<std::recursive_mutex> lck(*_staticMtx);

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
		ba.writeUnsignedShort(0x0100);
		ba.writeUnsignedChar(0);
		ba.writeString(error.c_str());
		c->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
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
				ba.writeUnsignedShort(0x0100);
				ba.writeUnsignedChar(3);
				ba.writeUnsignedInt(c->getID());
				ba.writeUnsignedChar(c->order);
				other->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
			}

			_clients.insert(std::pair<unsigned int, std::tr1::shared_ptr<Client>>(c->getID(), c->getSharedPtr()));
			if (_host == nullptr) {
				_host = c;
			}

			ByteArray ba(false);
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
			c->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
		}

		return "";
	}
	else {
		return "is closed";
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

			if (_battleState == BattleState::NONE) {
				if (_host == c) {
					_host = nullptr;

					for (auto& itr : _clients) {
						_host = itr.second.get();
						break;
					}

					if (_host != nullptr) {
						ByteArray ba(false);
						ba.writeUnsignedShort(0x0100);
						ba.writeUnsignedChar(5);
						ba.writeUnsignedInt(_host->getID());

						for (auto& itr : _clients) {
							Client* other = itr.second.get();
							other->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
						}
					}
				}

				ByteArray ba(false);
				ba.writeUnsignedShort(0x0100);
				ba.writeUnsignedChar(4);
				ba.writeUnsignedInt(c->getID());

				for (auto& itr : _clients) {
					Client* other = itr.second.get();
					other->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
				}


				ba.setLength(0);
				ba.writeUnsignedShort(0x0100);
				ba.writeUnsignedChar(2);
				c->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);

				_sendLevelSyncComplete();
			} else {
				if (_host == c) {
					_host = nullptr;
				}

				ByteArray ba(false);
				ba.writeUnsignedShort(0x0200);
				ba.writeUnsignedChar(5);
				ba.writeUnsignedInt(c->getID());

				for (auto& itr : _clients) {
					Client* other = itr.second.get();
					other->sendData(ba.getBytes(), ba.getLength(), NetType::KCP);
				}

				if (_host == nullptr && _battleState != BattleState::FINISHING) {
					_sendFinishState(3);
				}
			}
		}

		if (getNumClients() == 0) {
			close();
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
			ba.writeUnsignedShort(0x0100);
			ba.writeUnsignedChar(6);
			ba.writeUnsignedInt(c->getID());
			ba.writeBool(c->ready);
			c2->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
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
				c2->levelInited = 0;

				ByteArray ba(false);
				ba.writeUnsignedShort(0x0100);
				ba.writeUnsignedChar(7);
				ba.writeBool(true);
				c2->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
			}

			_battleState = BattleState::PRE_INIT;
		} else {
			ByteArray ba(false);
			ba.writeUnsignedShort(0x0100);
			ba.writeUnsignedChar(7);
			ba.writeBool(false);
			c->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
		}
	}
}

void Room::syncClient(Client* c, ByteArray* data) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	if (_battleState == BattleState::PRE_INIT) {
		if (c->levelInited == 0) {
			c->levelInited = 1;

			for (auto& itr : _clients) {
				Client* c2 = itr.second.get();
				if (c2 == c) continue;

				ByteArray ba(false);
				ba.writeUnsignedShort(0x0200);
				ba.writeUnsignedChar(0);
				ba.writeBytes(data);
				c2->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
			}

			_sendLevelSyncComplete();
		}
	}
}

void Room::syncEntity(Client* c, ByteArray* data) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	if (_battleState == BattleState::RUNNING) {
		for (auto& itr : _clients) {
			Client* c2 = itr.second.get();
			if (c2 == c) continue;

			ByteArray ba(false);
			ba.writeUnsignedShort(0x0200);
			ba.writeUnsignedChar(3);
			ba.writeBytes(data);
			c2->sendData(ba.getBytes(), ba.getLength(), NetType::KCP);
		}
	}
}

void Room::syncEntityHP(Client* c, ByteArray* data) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	if (_battleState == BattleState::RUNNING) {
		if (_host != nullptr) {
			ByteArray ba(false);
			ba.writeUnsignedShort(0x0200);
			ba.writeUnsignedChar(4);
			ba.writeBytes(data);
			_host->sendData(ba.getBytes(), ba.getLength(), NetType::KCP);
		}
	}
}

void Room::syncEntityGeneratorCreate(Client* c, ByteArray* data) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	if (_battleState == BattleState::RUNNING) {
		if (_host != nullptr) {
			ByteArray ba(false);
			ba.writeUnsignedShort(0x0200);
			ba.writeUnsignedChar(7);
			ba.writeUnsignedInt(data->readUnsignedInt());
			
			for (auto& itr : _clients) {
				Client* other = itr.second.get();
				if (other == c) continue;

				other->sendData(ba.getBytes(), ba.getLength(), NetType::KCP);
			}
		}
	}
}

void Room::initLevelComplete(Client* c) {
	std::lock_guard<std::recursive_mutex> lck(_mtx);

	if (_battleState == BattleState::INITING) {
		if (c->levelInited == 1) {
			c->levelInited = 2;

			_sendLevelSyncComplete();
		}
	}
}

bool Room::_isEqualInitState(unsigned int state) {
	for (auto& itr : _clients) {
		Client* c = itr.second.get();

		if (c->levelInited != state) {
			return false;
		}
	}

	return true;
}

void Room::_sendLevelSyncComplete() {
	if (_battleState != BattleState::RUNNING) {
		if (_battleState == BattleState::PRE_INIT && _isEqualInitState(1)) {
			_battleState = BattleState::INITING;

			ByteArray ba(false);
			ba.writeUnsignedShort(0x0200);
			ba.writeUnsignedChar(1);
			ba.writeUnsignedInt(_host == nullptr ? 0 : _host->getID());

			for (auto& itr : _clients) {
				Client* c = itr.second.get();
				c->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
			}
		} else if (_battleState == BattleState::INITING && _isEqualInitState(2)) {
			_battleState = BattleState::RUNNING;

			ByteArray ba(false);
			ba.writeUnsignedShort(0x0200);
			ba.writeUnsignedChar(2);

			for (auto& itr : _clients) {
				Client* c = itr.second.get();
				c->sendData(ba.getBytes(), ba.getLength(), NetType::TCP);
			}

			if (_host == nullptr) {
				_sendFinishState(3);
			}
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

			for (auto& itr : _clients) {
				Client* c = itr.second.get();
				c->exitRoom();
			}

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

void Room::_sendFinishState(unsigned char state) {
	_battleState = BattleState::FINISHING;

	ByteArray ba(false);
	ba.writeUnsignedShort(0x0200);
	ba.writeUnsignedChar(6);
	ba.writeUnsignedChar(state);

	for (auto& itr : _clients) {
		Client* c = itr.second.get();
		c->sendData(ba.getBytes(), ba.getLength(), NetType::KCP);
	}

	if (state == 3) {
		close();
	}
}
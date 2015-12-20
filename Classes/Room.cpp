#include "Room.h"
#include "Client.h"
#include "ByteArray.h"

Room::Room() {
	_id = _idAccumulator++;

	_host = nullptr;
	_isClosed = false;

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
		room->addClient(c);
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

			for (auto& itr : _clients) {
				Client* other = itr.second.get();

				ByteArray ba(false);
				ba.writeUnsignedShort(0);
				ba.writeUnsignedShort(0x0100);
				ba.writeUnsignedChar(3);
				ba.writeUnsignedInt(c->getID());
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
					ba.writeUnsignedInt(c->getID());
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
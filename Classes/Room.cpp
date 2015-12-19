#include "Room.h"
#include "Client.h"

Room::Room() {
	_id = _idAccumulator++;

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

void Room::addClient(Client* c) {
	if (!_isClosed) {
		std::lock_guard<std::recursive_mutex> lck(_mtx);

		auto& itr = _clients.find(c->getID());
		if (itr == _clients.end()) {
			_clients.insert(std::pair<unsigned int, std::tr1::shared_ptr<Client>>(c->getID(), c->getSharedPtr()));
		}
	}
}

void Room::removeClient(Client* c) {
	if (!_isClosed) {
		std::lock_guard<std::recursive_mutex> lck(_mtx);

		auto& itr = _clients.find(c->getID());
		if (itr != _clients.end()) {
			_clients.erase(itr);
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
#include "Room.h"
#include "Client.h"

Room::Room() {
	_id = _idAccumulator++;
}

Room::~Room() {
}

unsigned int Room::_idAccumulator = 1;
std::unordered_map<unsigned int, Room*> Room::_rooms = std::unordered_map<unsigned int, Room*>();

Room* Room::create() {
	Room* r = new Room();
	_rooms.insert(std::pair<unsigned int, Room*>(r->getID(), r));
	return r;
}

Room* Room::get(unsigned int id) {
	auto& itr = _rooms.find(id);
	return itr == _rooms.end() ? nullptr : itr->second;
}

void Room::addClient(unsigned int id) {
	auto& itr = _clients.find(id);
	if (itr == _clients.end()) {
		_clients.insert(std::pair<unsigned int, bool>(id, true));
	}
}

void Room::removeClient(unsigned int id) {
	auto& itr = _clients.find(id);
	if (itr != _clients.end()) {
		_clients.erase(itr);
	}
}
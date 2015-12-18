#pragma once

#include <unordered_map>

class Room {
public:
	Room();
	virtual ~Room();

	static Room* __fastcall create();
	static Room* __fastcall get(unsigned int id);

	inline unsigned int __fastcall getID() { return _id; }
	void __fastcall addClient(unsigned int id);
	void __fastcall removeClient(unsigned int id);

protected:
	static unsigned int _idAccumulator;
	static std::unordered_map<unsigned int, Room*> _rooms;

	unsigned int _id;
	std::unordered_map<unsigned int, bool> _clients;
};
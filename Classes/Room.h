#pragma once

#include <unordered_map>
#include <mutex>
#include <memory>

class Client;
class ByteArray;

class Room {
public:
	enum class BattleState {
		NONE,
		PRE_INIT,
		INITING,
		RUNNING
	};

	Room();
	virtual ~Room();

	static const unsigned int MAX_NUM_PLAYERS = 4;

	static Room* __fastcall create();
	static void __fastcall remove(unsigned int id);
	static Room* __fastcall get(unsigned int id);
	static void __fastcall joinRoom(Client* c, unsigned int id);

	inline unsigned int __fastcall getID() { return _id; }
	std::string  __fastcall addClient(Client* c);
	void __fastcall removeClient(Client* c);
	void __fastcall setClientReady(Client* c, bool b);
	void __fastcall startLevel(Client* c);
	void __fastcall syncClient(Client* c, ByteArray* ba);
	void __fastcall syncEntity(Client* c, ByteArray* ba);
	void __fastcall initLevelComplete(Client* c);
	virtual void __fastcall close();
	unsigned int __fastcall getNumClients();

	const std::tr1::shared_ptr<Room>& getSharedPtr() { return _self; }

protected:
	static unsigned int _idAccumulator;
	static std::recursive_mutex* _staticMtx;
	static std::unordered_map<unsigned int, Room*> _rooms;

	unsigned int _syncClients;
	unsigned int _clientsMask;
	unsigned int _id;
	bool _isClosed;
	BattleState _battleState;
	std::recursive_mutex _mtx;
	std::unordered_map<unsigned int, std::tr1::shared_ptr<Client>> _clients;

	std::tr1::shared_ptr<Room> _self;
	Client* _host;

	void __fastcall _sendLevelSyncComplete();
	char __fastcall _getEmptyClientOrder();
	void __fastcall _setClientOrderMask(unsigned char index, bool b);
};
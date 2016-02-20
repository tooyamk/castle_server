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
		RUNNING,
		FINISHING
	};

	Room();
	virtual ~Room();

	static const unsigned int MAX_NUM_PLAYERS = 4;

	static Room* __fastcall create();
	static void __fastcall remove(unsigned int id);
	static void __fastcall addToState(Room* room, BattleState oldState);
	static void __fastcall removeFromState(Room* room);
	static std::unordered_map<unsigned int, Room*>* __fastcall getMapFromState(BattleState state);
	static Room* __fastcall get(unsigned int id);
	static void __fastcall joinRoom(Client* c, unsigned int id);
	static void __fastcall matchRoom(Client* c);

	inline unsigned int __fastcall getID() { return _id; }
	std::string  __fastcall addClient(Client* c);
	void __fastcall removeClient(Client* c);
	void __fastcall setClientReady(Client* c, bool b);
	void __fastcall startLevel(Client* c);
	void __fastcall syncClient(Client* c, ByteArray* ba);
	void __fastcall syncEntity(Client* c, ByteArray* ba);
	void __fastcall syncEntityHP(Client* c, ByteArray* ba);
	void __fastcall syncEntityGeneratorCreate(Client* c, ByteArray* ba);
	void __fastcall initLevelComplete(Client* c);
	void __fastcall setBattleFinish(Client* c, ByteArray* ba);
	void __fastcall setGobackReadyRoom(Client* c);
	virtual void __fastcall close();
	unsigned int __fastcall getNumClients();
	inline BattleState __fastcall getBattleState() { return _battleState; }
	void __fastcall setBattleState(BattleState state);

	const std::tr1::shared_ptr<Room>& getSharedPtr() { return _self; }

protected:
	static unsigned int _idAccumulator;
	static std::recursive_mutex* _staticMtx;
	static std::unordered_map<unsigned int, Room*> _rooms;
	static std::unordered_map<unsigned int, Room*> _combatRooms;
	static std::unordered_map<unsigned int, Room*> _nocombatRooms;

	unsigned int _clientsMask;
	unsigned int _id;
	bool _isClosed;
	unsigned long long _battleStartTime;
	BattleState _battleState;
	std::recursive_mutex _mtx;
	std::unordered_map<unsigned int, std::tr1::shared_ptr<Client>> _clients;

	std::tr1::shared_ptr<Room> _self;
	Client* _host;

	void __fastcall _sendLevelSyncComplete();
	char __fastcall _getEmptyClientOrder();
	void __fastcall _setClientOrderMask(unsigned char index, bool b);
	bool __fastcall _isEqualInitState(unsigned int state);
	void __fastcall _sendFinishState(unsigned char state);
	void __fastcall _send0x0100_1(Client* c);//add new player init self
	void __fastcall _send0x0100_3(Client* c);//send add new player data to others
};
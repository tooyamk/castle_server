#pragma once

#include "NetWin32.h"
#include <unordered_map>

class NetDataBuffer;
class ByteArray;
class Room;
class Packet;

class Client {
public:
	Client(unsigned int socket, const sockaddr_in& addr);
	virtual ~Client();

	static void __fastcall addClient(Client* c);
	static void __fastcall remvoeClient(unsigned int id);

	inline unsigned int __fastcall getID() { return _id; }
	virtual void __fastcall run();

	Room* curRoom;

protected:
	static unsigned int _idAccumulator;
	static std::unordered_map<unsigned int, Client*> _clients;

	unsigned int _id;
	SocketWin32* _socket;
	NetDataBuffer* _socketReceiveBuffer;
	ByteArray* _socketReciveBytes;
	sockaddr_in _addr;

	void __fastcall _socketReceiveHandler();

	void __fastcall _receive0x0100(Packet* p);
	void __fastcall _receive0x0101(Packet* p);
};
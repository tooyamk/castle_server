#pragma once

#include "NetWin32.h"
#include <unordered_map>
#include <memory>
#include <mutex>
#include "NetDataBuffer.h"

extern "C" {
#include "ikcp.h"
}

class NetDataBuffer;
class ByteArray;
class Room;
class Packet;
class NetServer;

class Client {
public:
	Client(unsigned int socket, const sockaddr_in& addr, NetServer* server);
	virtual ~Client();

	static void __fastcall addClient(Client* c);
	static void __fastcall remvoeClient(unsigned int id);
	static std::tr1::shared_ptr<Client> __fastcall getClient(unsigned int id);

	inline unsigned int __fastcall getID() { return _id; }
	virtual void __fastcall receiveUDP(ByteArray* bytes, bool kcp, unsigned short len, sockaddr_in* addr);
	virtual void __fastcall run();
	virtual void __fastcall close();
	virtual void __fastcall exitRoom();
	virtual Room* __fastcall getCurRoom();
	virtual void __fastcall setCurRoom(Room* room);
	virtual void __fastcall switchReadyState();
	virtual void __fastcall startChapter();
	virtual void __fastcall syncClient(ByteArray* ba);
	virtual void __fastcall syncEntity(ByteArray* ba);
	virtual void __fastcall initChapterComplete();
	virtual void __fastcall sendData(const char* bytes, int len, NetType type);
	virtual void __fastcall __sendUDP(const char* data, unsigned int len);
	const std::tr1::shared_ptr<Client>& getSharedPtr() { return _self; }

	int order;
	bool ready;
	char levelInited;
	sockaddr_in* udpAddr;

protected:
	static std::recursive_mutex* _staticMtx;
	static unsigned int _idAccumulator;
	static std::unordered_map<unsigned int, Client*> _clients;

	NetServer* _server;
	std::tr1::shared_ptr<Room> _curRoom;
	bool _isClosed;
	std::recursive_mutex _mtx;
	std::recursive_mutex _sendMtx;
	unsigned int _id;
	std::tr1::shared_ptr<Client> _self;
	std::tr1::shared_ptr<Client> _selfTcp;
	std::tr1::shared_ptr<Client> _selfKcp;
	SocketWin32* _tcp;
	NetDataBuffer* _tcpReceiveBuffer;
	ByteArray* _tcpReciveBytes;
	sockaddr_in _addr;
	ikcpcb* _kcp;
	NetDataBuffer* _kcpSendBuffer;
	NetDataBuffer* _kcpReceiveBuffer2;
	IUINT32 _nextUpdateKCPTime;
	char _udpReceiveToKcpBuffer[NetDataBuffer::BufferNode::MAX_LEN];
	char _kcpReceiveBuffer[NetDataBuffer::BufferNode::MAX_LEN];
	char _tcpSendBuffer[NetDataBuffer::BufferNode::MAX_LEN];

	void __fastcall _tcpReceiveHandler();
	void __fastcall _kcpHandler();
	void __fastcall _executePacket(Packet* p);
};
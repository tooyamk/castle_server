#pragma once

#include "ByteArray.h"

class Packet {
public:
	Packet();
	virtual ~Packet();

	unsigned int clientID;
	unsigned short head;
	ByteArray bytes;

	static unsigned int parse(ByteArray* bytes, Packet* p, bool udp = false);
};
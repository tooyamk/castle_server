#pragma once

#include "ByteArray.h"

class Packet {
public:
	Packet();
	virtual ~Packet();

	unsigned short head;
	ByteArray bytes;

	static unsigned int parse(ByteArray* bytes, Packet* p);
};
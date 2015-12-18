#include "Packet.h"

Packet::Packet() :
bytes(false, 0){
}

Packet::~Packet() {
}

unsigned int Packet::parse(ByteArray* bytes, Packet* p) {
	unsigned int readSize = 0;

	if (bytes->getBytesAvailable() >= 2) {
		unsigned int size = bytes->readUnsignedShort();
		if (bytes->getBytesAvailable() >= size) {
			readSize += size + 2;

			p->head = bytes->readUnsignedShort();
			if (size > 2) bytes->readBytes(&p->bytes, 0, size - 2);
		}
	}

	return readSize;
}
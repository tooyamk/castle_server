#include "Packet.h"

Packet::Packet() :
bytes(false, 0){
	clientID = 0;
}

Packet::~Packet() {
}

unsigned int Packet::parse(ByteArray* bytes, Packet* p, bool udp) {
	unsigned int readSize = 0;

	if (bytes->getBytesAvailable() >= 2) {
		unsigned int size = bytes->readUnsignedShort();
		if (bytes->getBytesAvailable() >= size) {
			readSize += size + 2;

			unsigned int pos = bytes->getPosition();

			if (udp) p->clientID = bytes->readUnsignedInt();
			p->head = bytes->readUnsignedShort();

			unsigned int n = bytes->getPosition() - pos;
			if (size > n) bytes->readBytes(&p->bytes, 0, size - n);
		}
	}

	return readSize;
}
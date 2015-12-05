#include "BaseUDP.h"

BaseUDP::BaseUDP() {
}

BaseUDP::~BaseUDP() {
}

void BaseUDP::start(char* addr, int port) {
}

void BaseUDP::send(const char* buf, int len, sockaddr* addr) {
}

int BaseUDP::receive(char* buf, int len, sockaddr* addr) {
	return -1;
}

UDPState BaseUDP::getState() {
	return UDPState::DISCONNECTED;
}

void BaseUDP::close() {
}
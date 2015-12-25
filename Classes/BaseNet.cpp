#include "BaseNet.h"

BaseNet::BaseNet() {
}

BaseNet::~BaseNet() {
}

void BaseNet::start(char* addr, int port) {
}

void BaseNet::start(unsigned int s) {
}

void BaseNet::close() {
}

void BaseNet::sendData(const char* buf, int len, sockaddr* addr) {
}

int BaseNet::receiveData(char* buf, int len, sockaddr* addr) {
	return -1;
}

unsigned int BaseNet::acceptClient(sockaddr* addr, int* addrLen) {
	return 0;
}

ConnectState BaseNet::getState() {
	return ConnectState::DISCONNECTED;
}
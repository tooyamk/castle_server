#include "Server.h"

Server::Server() {
	_net = new NetServer();
}

Server::~Server() {
}

void Server::run() {
	_net->run();
}
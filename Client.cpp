#include "Client.hpp"

Client::Client(int clientFd) : fd(clientFd), authenticated(false), authState(0) { }

Client::~Client() { }

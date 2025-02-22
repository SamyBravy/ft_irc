#include "Client.hpp"

Client::Client(int clientFd) : fd(clientFd), authenticated(false) { }

Client::~Client() { }

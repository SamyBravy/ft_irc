#include "Channel.hpp"

Channel::Channel(Server *server, const std::string &name) : _name(name), _server(server) { }

std::vector<Client *> Channel::getClients() const
{
    std::vector<Client *> clients;

    for (std::map<std::string, ChannelClient>::const_iterator it = _users.begin(); it != _users.end(); it++)
        clients.push_back(it->second.client);

    return clients;
}

bool Channel::userExists(const std::string &nickname) const
{
    return _users.find(nickname) != _users.end();
}

void Channel::sendMsg(const std::string &msg) const
{
    if (_server == NULL)
        return;
    for (std::map<std::string, ChannelClient>::const_iterator it = _users.begin(); it != _users.end(); it++)
        _server->sendMsg(it->second.client->fd, msg);
}

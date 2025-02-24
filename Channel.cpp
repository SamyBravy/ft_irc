#include "Channel.hpp"

Channel::Channel(Server *server, const std::string &name) : _server(server), _name(name), _limit(0),
                                                                _operatorsCount(0), _topicProtected(false), _inviteOnly(false)
{
    _creationMoment = time(NULL);
}

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

void Channel::sendMsg(const std::string &msg, const std::string &notToSend) const
{
    if (_server == NULL)
        return;
    for (std::map<std::string, ChannelClient>::const_iterator it = _users.begin(); it != _users.end(); it++)
    {
        if (it->first == notToSend)
            continue;
        _server->sendMsg(it->second.client->fd, msg);
    }
}

void Channel::updateNick(const std::string &oldNick, const std::string &newNick)
{
    if (_users.find(oldNick) != _users.end())
    {
        ChannelClient channelClient = _users[oldNick];
        _users.erase(oldNick);
        _users[newNick] = channelClient;
    }
    if (_invited.find(oldNick) != _invited.end())
    {
        _invited.erase(oldNick);
        _invited.insert(newNick);
    }
    if (_topic.author == oldNick)
        _topic.author = newNick;
}

void Channel::setInviteOnly(bool inviteOnly)
{
    _inviteOnly = inviteOnly;
    if (!_inviteOnly)
        _invited.clear();
}

void Channel::setTopic(const std::string &topic, const std::string &author)
{
    _topic.str = topic;
    _topic.author = author;
    _topic.time = time(NULL);
}

bool Channel::isInvited(const std::string &nickname) const
{
    return !_inviteOnly || _invited.find(nickname) != _invited.end();
}

bool Channel::isOperator(const std::string &nickname) const
{
    return _users.find(nickname) != _users.end() && _users.at(nickname).isOperator;
}

bool Channel::passMatch(const std::string &key) const
{
    return _password == key || _password.empty();
}

void Channel::addClient(Client *client, bool isOperator)
{
    ChannelClient channelClient = { client, isOperator };
    _users[client->nickname] = channelClient;
    if (isOperator)
        _operatorsCount++;

    if (_invited.find(client->nickname) != _invited.end())
        _invited.erase(client->nickname);
}

void Channel::addInvited(const std::string &nickname)
{
    _invited.insert(nickname);
}

void Channel::removeClient(const std::string &nickname)
{
    if (_users.find(nickname) == _users.end())
        return;

    Client *client = _users[nickname].client;
    std::string newOpMsg = ":" + client->nickname + "!" + client->username + "@" + client->hostname + " MODE " + _name + " -o ";

    if (isOperator(nickname))
        _operatorsCount--;
    _users.erase(nickname);
    _invited.erase(nickname);

    if (!_users.empty() && _operatorsCount == 0)
    {
        std::map<std::string, ChannelClient>::iterator it = _users.begin();
        std::advance(it, rand() % _users.size());
        setOperator(it->first);
        sendMsg(newOpMsg + it->first);
    }
}

void Channel::setOperator(const std::string &nickname)
{
    if (_users.find(nickname) != _users.end())
    {
        _users[nickname].isOperator = true;
        _operatorsCount++;
    }
}

void Channel::unsetOperator(const std::string &nickname)
{
    if (_users.find(nickname) != _users.end())
    {
        _users[nickname].isOperator = false;
        _operatorsCount--;
    }
}

std::string Channel::getMode() const
{
    if (!_inviteOnly && !_topicProtected && _password.empty() && !_limit)
        return "none";
    
    return std::string("+") + (_inviteOnly ? "i" : "") + (_topicProtected ? "t" : "") + (!_password.empty() ? "k" : "") + (_limit ? "l" : "");
}

std::string Channel::getNames() const
{
    std::string names;

    for (std::map<std::string, ChannelClient>::const_iterator it = _users.begin(); it != _users.end(); it++)
    {
        if (it->second.isOperator)
            names += "@";
        names += it->first + " ";
    }

    return names;
}

std::string Channel::getTopicTime() const
{
    std::ostringstream oss;

    oss << _topic.time;
    
    return oss.str();
}

std::string Channel::getCreationMoment() const
{
    std::ostringstream oss;

    oss << _creationMoment;
    
    return oss.str();
}

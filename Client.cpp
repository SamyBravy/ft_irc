#include "Client.hpp"

Client::Client(int clientFd) : fd(clientFd), authenticated(false) { }

std::string Client::getInfo() const
{
    return (username + " " + hostname + " " + servername + " " + nickname + " H :0 " + realname);
}

void Client::setNickname(const std::string &nick)
{
    for (std::vector<Channel *>::iterator it = channels.begin(); it != channels.end(); it++)
        (*it)->updateNick(nickname, nick);
    this->nickname = nick;
}

void Client::joinChannel(Channel *channel, bool isOperator)
{
    if (channel->userExists(nickname))
        return;
    channel->addClient(this, isOperator);
    channels.push_back(channel);
}

void Client::leaveChannel(Channel *channel)
{
    channel->removeClient(nickname);
    for (std::vector<Channel *>::iterator it = channels.begin(); it != channels.end(); it++)
    {
        if (*it == channel)
        {
            channels.erase(it);
            return;
        }
    }
}

#include "ft_irc.hpp"
#include "Client.hpp"

std::string Client::getInfo() const
{
    return (username + " " + hostname + " " + servername + " " + nickname + " H :0 " + realname);
}

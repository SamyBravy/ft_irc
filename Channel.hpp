#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "ft_irc.hpp"
#include "Client.hpp"
#include "Server.hpp"

struct ChannelClient
{
    Client *client;
    bool isOperator;
};

class Server;

class Channel
{
	private:
		std::string _name;
        std::string _password;
        std::string _topic;
        int _limit;
        bool _inviteOnly;
        bool _topicProtected;
        std::map<std::string, ChannelClient> _users;
        Server *_server;

	public:
		Channel(Server *server = NULL, const std::string &name = "default");
		~Channel() { }

        std::vector<Client *> getClients() const;
        bool userExists(const std::string &nickname) const;
        void sendMsg(const std::string &msg) const;
};

#endif

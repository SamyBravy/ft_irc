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
        Server *_server;
		std::string _name;
        std::string _password;
        std::string _topic;
        int _limit;
        bool _inviteOnly;
        bool _topicProtected;
        std::map<std::string, ChannelClient> _users;
        std::set<std::string> _invited;

	public:
		Channel(Server *server = NULL, const std::string &name = "default");
		~Channel() { }

        void updateNick(const std::string &oldNick, const std::string &newNick);
        std::vector<Client *> getClients() const;
        bool userExists(const std::string &nickname) const;
        std::string getName() const { return _name; }
        std::string getMode() const;
        int getLimit() const { return _limit; }
        int getUsersSize() const { return _users.size(); }
        std::string getTopic() const;
        std::string getNames() const;
        bool isInvited(const std::string &nickname) const;
        bool passMatch(const std::string &key) const;
        void setInviteOnly(bool inviteOnly);
        void addClient(Client *client, bool isOperator = false);
        void removeClient(const std::string &nickname);

        void sendMsg(const std::string &msg, const std::string &notToSend = "") const;
};

#endif

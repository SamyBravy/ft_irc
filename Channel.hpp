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

struct Topic
{
    std::string str;
    std::string author;
    time_t time;
};

class Server;

class Channel
{
	private:
        Server *_server;
        time_t _creationMoment;
		std::string _name;
        std::string _password;
        Topic _topic;
        int _limit;
        int _operatorsCount;
        bool _topicProtected;
        bool _inviteOnly;
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
        std::string getTopic() const { return _topic.str; }
        std::string getTopicAuthor() const { return _topic.author; }
        std::string getTopicTime() const;
        std::string getNames() const;
        std::string getCreationMoment() const;
        bool isInvited(const std::string &nickname) const;
        bool isOperator(const std::string &nickname) const;
        bool isInviteOnly() const { return _inviteOnly; }
        bool isTopicProtected() const { return _topicProtected; }
        bool passMatch(const std::string &key) const;
        void setInviteOnly(bool inviteOnly);
        void setTopicProtected(bool topicProtected) { _topicProtected = topicProtected; }
        void setPassword(const std::string &pass) { _password = pass; }
        void setLimit(int limit) { _limit = limit; }
        void setTopic(const std::string &topic, const std::string &author);
        void setOperator(const std::string &nickname);
        void unsetOperator(const std::string &nickname);
        void addClient(Client *client, bool isOperator = false);
        void addInvited(const std::string &nickname);
        void removeClient(const std::string &nickname);

        void sendMsg(const std::string &msg, const std::string &notToSend = "") const;
};

#endif

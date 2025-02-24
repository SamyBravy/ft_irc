#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ft_irc.hpp"
# include "Channel.hpp"

class Channel;

struct Client
{
	int fd;
	bool authenticated;
    std::string password;
	std::string nickname;
	std::string username;
    std::string hostname;
    std::string servername;
	std::string realname;
    std::string buffer;
    std::vector <Channel *> channels;
	
	Client(int clientFd = -1);
	~Client() { }

    void setNickname(const std::string &nick);
    void joinChannel(Channel *channel, bool isOperator = false);
    void leaveChannel(Channel *channel);

    bool isInAChannelWith(const std::string &nick) const;

    std::string getInfo() const;
};

#endif

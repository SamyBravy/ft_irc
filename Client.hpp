#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ft_irc.hpp"

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
	
	Client(int clientFd = -1) : fd(clientFd), authenticated(false) { }
	~Client() { }

    std::string getInfo() const;
};

#endif

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
	
	Client(int clientFd = -1);
	~Client();
};

#endif

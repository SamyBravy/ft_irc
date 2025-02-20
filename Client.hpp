#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ft_irc.hpp"

struct Client
{
	int fd;
	bool authenticated;
    short int authState;
    std::string password;
	std::string nickname;
	std::string username;
	std::string realname;
    std::string hostname;
    std::string servername;
	
	Client(int clientFd = -1);
	~Client();
};

#endif

#include "Server.hpp"

Server::Server(int port, const std::string &password) : _fd(-1), _port(port), _password(password) { }

Server::~Server()
{
	if (_fd != -1)
		close(_fd);
	
	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i].fd != -1)
			close(_clients[i].fd);
	}
}

void Server::run()
{
	createSocket();
	bindServer();
	listenServer();
}

void Server::sendMsg(int fd, std::string msg) const
{
    msg += "\r\n";

    if (fd == -1)
    {
        for (size_t i = 0; i < _clients.size(); i++)
        {
            if (_clients[i].fd != -1)
            {
                if (send(_clients[i].fd, msg.c_str(), msg.size(), 0) == -1)
                    std::cerr << "Error sending message" << std::endl;
            }
        }
    }
    else if (send(fd, msg.c_str(), msg.size(), 0) == -1)
        std::cerr << "Error sending message" << std::endl;
}

void Server::listenClients()
{
    while (1)
	{
		struct pollfd *fds = getPollfds();
		
		int ret = poll(fds, MAX_CLIENTS + 1, -1);
		if (ret > 0)
		{
			if (fds[0].revents & POLLIN)
			{
				int client_fd = accept(_fd, NULL, NULL);
				if (client_fd != -1)
                {
                    try
                    {
                        addClient(client_fd);
                        std::cout << "New client connected: " << client_fd << std::endl;
                    }
                    catch (const Server::ServerException &e)
                    {
                        std::cerr << e.what() << std::endl;
                        close(client_fd);
                    }
                }
				else
					std::cerr << "Error accepting new client" << std::endl;
			}
			
			for (size_t i = 1; i <= _clients.size() && fds[i].fd != -1; i++)
			{
				if (fds[i].revents & POLLIN)
				{
					char message[1024];
					int n = recv(fds[i].fd, message, sizeof(message) - 1, 0);
					if (n > 0)
					{
                        message[n] = '\0';
                        if (message[n - 1] != '\n')
                        {
                            _clients[i - 1].buffer += message;
                            continue;
                        }
                        std::string messageStr(_clients[i - 1].buffer + message);
                        messageStr.erase(messageStr.size() - 1);
                        _clients[i - 1].buffer.clear();
                        
                        messageStr.erase(std::remove(messageStr.begin(), messageStr.end(), '\r'), messageStr.end());
                        if (messageStr.empty())
                            continue;
                        if (_clients[i - 1].nickname.empty())
                            std::cout << "Client " << fds[i].fd << ": \"" << messageStr << "\"" << std::endl;
						else
                            std::cout << _clients[i - 1].nickname << ": \"" << messageStr << "\"" << std::endl;
                        handleMessage(messageStr, i - 1);
					}
					else if (n == 0)
					{
                        if (_clients[i - 1].nickname.empty())
						    std::cout << "Client " << fds[i].fd << " disconnected" << std::endl;
                        else
                            std::cout << _clients[i - 1].nickname << " disconnected" << std::endl;
						removeClient(i - 1);
					}
					else
					{
                        if (_clients[i - 1].nickname.empty())
						    std::cerr << "Error receiving data from client " << fds[i].fd << std::endl;
                        else
                            std::cerr << "Error receiving data from " << _clients[i - 1].nickname << std::endl;
					}
				}
			}
		}
		else if (ret == -1)
		{
			throw Server::ServerException("Error polling sockets");
		}
	}
}

struct pollfd *Server::getPollfds() const
{
	static struct pollfd fds[MAX_CLIENTS + 1];

	fds[0].fd = _fd;
	fds[0].events = POLLIN;

	for (size_t i = 1; i <= _clients.size(); i++)
	{
		fds[i].fd = _clients[i - 1].fd;
		fds[i].events = POLLIN;
	}

	for (size_t i = _clients.size() + 1; i <= MAX_CLIENTS; i++)
		fds[i].fd = -1;

	return fds;
}

void Server::addClient(int client_fd)
{
    if (_clients.size() >= MAX_CLIENTS)
    {
        throw ServerException("Error: too many clients");
    }
    
	_clients.push_back(Client(client_fd));
}

void Server::removeClient(int i)
{
    if (i < 0 || i >= (int)_clients.size())
        throw ServerException("Error: invalid client index");
    
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
        it->second.removeClient(_clients[i].nickname);
    if (_clients[i].fd != -1)
    {
        close(_clients[i].fd);
    }
	_clients.erase(_clients.begin() + i);
}

void Server::passCommand(const std::string &message, Client &client)
{
    std::string pass = getWord(message, 1);

    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + std::string("PASS") + ERR_NEEDMOREPARAMS);
    else if (pass != _password)
        sendMsg(client.fd, PREFIX_ERR_PASSWDMISMATCH + std::string("PASS") + ERR_PASSWDMISMATCH);
    else
        client.password = pass;
}

void Server::nickCommand(const std::string &message, Client &client)
{
    std::string nick = getWord(message, 1);

    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NONICKNAMEGIVEN + client.nickname + " NICK" + ERR_NONICKNAMEGIVEN);
    else if (message.find_first_of(":@#&") != std::string::npos)
        sendMsg(client.fd, PREFIX_ERR_ERRONEUSNICKNAME + client.nickname + " NICK " + nick + ERR_ERRONEUSNICKNAME);
    else if (clientExists(nick))
    {
        if (client.authenticated == false)
            sendMsg(client.fd, PREFIX_ERR_NICKNAMEINUSE + std::string(" NICK ") + nick + ERR_NICKNAMEINUSE);
        else
            sendMsg(client.fd, PREFIX_ERR_NICKNAMEINUSE + client.nickname + " " + nick + ERR_NICKNAMEINUSE);
    }
    else
    {
        if (client.authenticated == true)
            sendMsg(-1, ":" + client.nickname + "!" + client.username + "@" + client.hostname + " NICK :" + nick);
        client.setNickname(nick);
    }
}

void Server::userCommand(const std::string &message, Client &client)
{
    if (countWords(message) < 5)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " USER" + ERR_NEEDMOREPARAMS);
    else
    {
        client.username = getWord(message, 1);
        client.hostname = getWord(message, 2);
        client.servername = getWord(message, 3);
        if (getWord(message, 4).find_first_of(":") == 0)
        {
            client.realname = getWord(message, 4).substr(1);
            for (int i = 5; i < countWords(message); i++)
                client.realname += " " + getWord(message, i);
        }
        else
            client.realname = getWord(message, 4);
    }
}

void Server::pingCommand(const std::string &message, Client &client) const
{
    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " PING" + ERR_NEEDMOREPARAMS);
    else
        sendMsg(client.fd, std::string(":ft_irc ") + " PONG ft_irc " + getWord(message, 1));
}

void Server::sendWelcomeMessage(const Client &client) const
{
    sendMsg(client.fd, ":ft_irc 001 " + client.nickname + " :Welcome to the Manicomio Network, "
        + client.nickname + "!" + client.username + "@" + client.hostname);
    sendMsg(client.fd, ":ft_irc 002 " + client.nickname + " :Your host is ft_irc, running version 1.0");
    sendMsg(client.fd, ":ft_irc 003 " + client.nickname + " :This server was created "
        + getDay(_creationMoment) + ", " + getDate(_creationMoment) + " at " + getTime(_creationMoment));
    sendMsg(client.fd, ":ft_irc 004 " + client.nickname + " " + client.servername + " 1.0 uMode:none cMode:+i,+t,+k,+o,+l");
    sendMsg(client.fd, ":ft_irc 375 " + client.nickname + " :- Welcome to the Manicomio");
    sendMsg(client.fd, ":ft_irc 372 " + client.nickname + " :- Eat my Desktop Background");
    sendMsg(client.fd, ":ft_irc 376 " + client.nickname + " :- End of /MOTD command");
}

void Server::capCommand(const std::string &message, Client &client) const
{
    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " CAP" + ERR_NEEDMOREPARAMS);
    else if (message == "CAP LS 302")
    {
        std::string capabilities = "";
        sendMsg(client.fd, "CAP * LS :" + capabilities);
    }
}

void Server::whoCommand(const std::string &message, Client &client)
{
    std::string name = getWord(message, 1);

    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " WHO" + ERR_NEEDMOREPARAMS);
    else if (!clientExists(name) && !channelExists(name))
        sendMsg(client.fd, PREFIX_ERR_NOSUCHNICK + client.nickname + " " + name + ERR_NOSUCHNICK);
    else
    {
        if (clientExists(name))
            sendMsg(client.fd, ":ft_irc 352 " + client.nickname
                    + (client.channels.size() > 0 ? " " + client.channels[0]->getName()  : " * ") + getClient(name).getInfo()); 
        else
        {
            std::vector<Client *> channelClients = _channels[name].getClients();
            for (size_t i = 0; i < channelClients.size(); i++)
                sendMsg(client.fd, ":ft_irc 352 " + client.nickname + " " + name + " " + channelClients[i]->getInfo());
        }
        sendMsg(client.fd, ":ft_irc 315 " + client.nickname + " " + name + " :End of /WHO list");
    }
}

void Server::joinCommand(const std::string &message, Client &client)
{
    std::string channelName = getWord(message, 1);

    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " JOIN" + ERR_NEEDMOREPARAMS);
    else if (channelName[0] != '#' && channelName[0] != '&')
        sendMsg(client.fd, PREFIX_ERR_NOSUCHCHANNEL + client.nickname + " " + channelName + ERR_NOSUCHCHANNEL);
    else
    {
        std::vector<std::string> channelsStr;
        std::vector<std::string> keysStr;
        std::stringstream ss(message);
        std::string token;
        ss >> token;
        while (ss >> token && token.find_first_of("#&") == 0)
            channelsStr.push_back(token);
        while (ss >> token)
            keysStr.push_back(token);
        
        for (size_t i = 0; i < channelsStr.size(); i++)
        {
            channelName = channelsStr[i];
            if (channelExists(channelName) && _channels[channelName].getLimit()
                && _channels[channelName].getUsersSize() >= _channels[channelName].getLimit())
                sendMsg(client.fd, PREFIX_ERR_CHANNELISFULL + client.nickname + " " + channelName + ERR_CHANNELISFULL);
            else if (channelExists(channelName) && !_channels[channelName].isInvited(client.nickname))
                sendMsg(client.fd, PREFIX_ERR_INVITEONLYCHAN + client.nickname + " " + channelName + ERR_INVITEONLYCHAN);
            else if (channelExists(channelName) && keysStr.size() > i && !_channels[channelName].passMatch(keysStr[i]))
                sendMsg(client.fd, PREFIX_ERR_BADCHANNELKEY + client.nickname + " " + channelName + ERR_BADCHANNELKEY);
            else
            {
                if(!channelExists(channelName))
                {
                    _channels[channelName] = Channel(this, channelName);
                    client.joinChannel(&_channels[channelName], true);
                    sendMsg(client.fd, ":" + client.nickname + "!ft_irc" + "@" + _ip + " JOIN " + channelName);
                }
                else
                {
                    client.joinChannel(&_channels[channelName]);
                    _channels[channelName].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname + " JOIN " + channelName);
                    sendMsg(client.fd, ":ft_irc 324 " + client.nickname + " " + channelName + " " + _channels[channelName].getMode());
                    sendMsg(client.fd, ":ft_irc 331 " + client.nickname + " " + channelName + " :" + _channels[channelName].getTopic());
                }
                sendMsg(client.fd, ":ft_irc 353 " + client.nickname + " = " + channelName + " :" + _channels[channelName].getNames());
                sendMsg(client.fd, ":ft_irc 366 " + client.nickname + " " + channelName + " :End of /NAMES list");
            }
        }
    }
}

void Server::privmsgCommand(const std::string &message, Client &client)
{
    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " PRIVMSG" + ERR_NEEDMOREPARAMS);
    else if (getWord(message, 1).find_first_of(":") == 0)
        sendMsg(client.fd, PREFIX_ERR_NORECIPIENT + client.nickname + " PRIVMSG" + ERR_NORECIPIENT);
    else if (message.find_first_of(":") == std::string::npos)
        sendMsg(client.fd, PREFIX_ERR_NOTEXTTOSEND + client.nickname + " PRIVMSG" + ERR_NOTEXTTOSEND);
    else
    {
        std::stringstream ss(message);
        std::string target;
        ss >> target;
        while (ss >> target && target[0] != ':')
        {
            if (!clientExists(target) && !channelExists(target))
                sendMsg(client.fd, PREFIX_ERR_NOSUCHNICK + client.nickname + " " + target + ERR_NOSUCHNICK);
            else
            {
                std::string toSendMsg = ":" + client.nickname + "!" + client.username + "@" + client.hostname + " PRIVMSG "
                    + target + " " + message.substr(message.find_first_of(":"));
                if (clientExists(target))
                    sendMsg(getClient(target).fd, toSendMsg);
                else
                {
                    if (_channels[target].userExists(client.nickname) == false)
                        sendMsg(client.fd, PREFIX_ERR_CANNOTSENDTOCHAN + client.nickname + " " + target + ERR_CANNOTSENDTOCHAN);
                    else
                        _channels[target].sendMsg(toSendMsg, client.nickname);
                }
            }
        }
    }
    
}

void Server::modeCommand(const std::string &message, Client &client)
{
    std::string target = getWord(message, 1);

    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " MODE" + ERR_NEEDMOREPARAMS);
    else if (countWords(message) == 2)
    {
        if (channelExists(target))
            sendMsg(client.fd, ":ft_irc 324 " + client.nickname + " " + target + " " + _channels[target].getMode());
        else if (target.find_first_of("#&") == 0)
            sendMsg(client.fd, PREFIX_ERR_NOSUCHCHANNEL + client.nickname + " " + target + ERR_NOSUCHCHANNEL);
        else if (!clientExists(target))
            sendMsg(client.fd, PREFIX_ERR_NOSUCHNICK + client.nickname + " " + target + ERR_NOSUCHNICK);
    }
}

void Server::handleMessage(std::string message, int iClient)
{
    if (iClient < 0 || iClient >= (int)_clients.size())
    {
        throw ServerException("Error: invalid client index");
    }

    Client &client = _clients[iClient];

    if (client.authenticated == false)
    {
        std::vector<std::string> tokens = split(message, '\n');

        for (size_t i = 0; i < tokens.size(); i++)
        {
            std::string command = getWord(tokens[i], 0);

            if (command == "CAP")
                capCommand(tokens[i], client);
            else if (command == "PASS" && client.password.empty())
                passCommand(tokens[i], client);
            else if (client.password.empty())
                sendMsg(client.fd, PREFIX_ERR_CUSTOM + std::string(":You must send a password first"));
            else if (command == "NICK" && client.nickname.empty())
                nickCommand(tokens[i], client);
            else if (command == "USER" && client.username.empty())
                userCommand(tokens[i], client);
            else if (client.nickname.empty())
                sendMsg(client.fd, PREFIX_ERR_CUSTOM + client.nickname + " :You must set a nickname first");
            else
                sendMsg(client.fd, PREFIX_ERR_CUSTOM + client.nickname + " :You must set a username first");

            if (!client.password.empty() && !client.nickname.empty() && !client.username.empty())
            {
                client.authenticated = true;
                sendWelcomeMessage(client);
                break;
            }
        }

        return;
    }

    std::string command = getWord(message, 0);

    if (command == "CAP")
        capCommand(message, client);
    else if (command == "PASS" || command == "USER")
        sendMsg(client.fd, PREFIX_ERR_ALREADYREGISTRED + client.nickname + ERR_ALREADYREGISTRED);
    else if (command == "NICK")
        nickCommand(message, client);
    else if (command == "PING")
        pingCommand(message, client);
    else if (command == "WHO")
        whoCommand(message, client);
    else if (command == "JOIN")
        joinCommand(message, client);
    else if (command == "PRIVMSG")
        privmsgCommand(message, client);
    else if (command == "MODE")
        modeCommand(message, client);
    else
        sendMsg(client.fd, PREFIX_ERR_UNKNOWNCOMMAND + client.nickname + " " + command + ERR_UNKNOWNCOMMAND);
}

void Server::createSocket()
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		throw ServerException("Error creating socket");
    _creationMoment = time(NULL);
}

void Server::bindServer()
{
	struct sockaddr_in _server_addr;

	memset(&_server_addr, 0, sizeof(_server_addr));
	_server_addr.sin_family = AF_INET;
	_server_addr.sin_addr.s_addr = INADDR_ANY;
	_server_addr.sin_port = htons(_port);

	int opt = 1;
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(_fd, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) < 0)
	{
		close(_fd);
		throw ServerException("Error binding server");
	}

    _ip = inet_ntoa(_server_addr.sin_addr);
}

void Server::listenServer()
{
	if (listen(_fd, MAX_CLIENTS) < 0)
	{
		close(_fd);
		throw ServerException("Error listening server");
	}

	std::cout << "Server listening on port " << _port << " with ip address " << _ip << std::endl;
}

bool Server::clientExists(const std::string &nickname) const
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i].nickname == nickname)
            return true;
    }
    return false;
}

Client &Server::getClient(const std::string nickname)
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i].nickname == nickname)
            return _clients[i];
    }
    throw ServerException("Error: client not found");
}

bool Server::channelExists(const std::string &channelName) const
{
    return _channels.find(channelName) != _channels.end();
}

Server::ServerException::ServerException(const char *msg) : _msg(msg) { }

const char *Server::ServerException::what() const throw()
{
	return _msg;
}

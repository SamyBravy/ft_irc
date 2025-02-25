#include "Server.hpp"

Server::Server(int port, const std::string &password) : _fd(-1), _port(port), _password(password)
{
    initCommandsAndModes();
    _clients.reserve(100);
}

Server::~Server()
{
    if (_fd != -1)
    {
        close(_fd);
    }
	
	for (size_t i = 0; i < _clients.size(); i++)
	{
        if (_clients[i].fd != -1)
        close(_clients[i].fd);
	}
}

void Server::initCommandsAndModes()
{
    _commands["CAP"] = &Server::capCommand;
    _commands["PASS"] = &Server::passCommand;
    _commands["NICK"] = &Server::nickCommand;
    _commands["USER"] = &Server::userCommand;
    _commands["PING"] = &Server::pingCommand;
    _commands["WHO"] = &Server::whoCommand;
    _commands["JOIN"] = &Server::joinCommand;
    _commands["PRIVMSG"] = &Server::privmsgCommand;
    _commands["NOTICE"] = &Server::noticeCommand;
    _commands["MODE"] = &Server::modeCommand;
    _commands["QUIT"] = &Server::quitCommand;
    _commands["PART"] = &Server::partCommand;
    _commands["KICK"] = &Server::kickCommand;
    _commands["INVITE"] = &Server::inviteCommand;
    _commands["TOPIC"] = &Server::topicCommand;

    _modes['i'] = &Server::inviteOnlyMode;
    _modes['t'] = &Server::topicProtectedMode;
    _modes['k'] = &Server::passwordMode;
    _modes['l'] = &Server::limitMode;
    _modes['o'] = &Server::operatorMode;
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
		std::vector<struct pollfd> fds = getPollfds();
		
		int ret = poll(fds.data(), fds.size(), -1);
		if (ret > 0)
		{
			if (fds[0].revents & POLLIN)
			{
				int client_fd = accept(_fd, NULL, NULL);
				if (client_fd != -1)
                {
                    try
                    {
                        _clients.push_back(Client(client_fd));
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
			
			for (size_t i = 1; i < fds.size(); i++)
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
                        if (!messageStr.empty() && messageStr[messageStr.size() - 1] == ' ')
                            messageStr.erase(messageStr.size() - 1);                         
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
                        quitCommand("QUIT", _clients[i - 1]);
                        removeEmptyChannels();
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

std::vector<struct pollfd> Server::getPollfds() const
{
    std::vector<struct pollfd> fds(_clients.size() + 1);

	fds[0].fd = _fd;
	fds[0].events = POLLIN;

	for (size_t i = 1; i <= _clients.size(); i++)
	{
		fds[i].fd = _clients[i - 1].fd;
		fds[i].events = POLLIN;
	}

	return fds;
}

void Server::removeClient(Client *client)
{
    if (client == NULL)
        throw ServerException("Error: invalid client");

    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
        it->second.removeClient(client->nickname);
    if (client->fd != -1)
    {
        close(client->fd);
    }
    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (&(*it) == client)
        {
            _clients.erase(it);
            break;
        }
    }
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
    else if (clientExists(nick) || nick == "ft_irc")
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
        if (getWord(message, 4).find(':') == 0)
        {
            client.realname = getWord(message, 4).substr(1);
            for (int i = 5; i < countWords(message); i++)
                client.realname += " " + getWord(message, i);
        }
        else
            client.realname = getWord(message, 4);
    }
}

void Server::pingCommand(const std::string &message, Client &client)
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
    sendMsg(client.fd, ":ft_irc 003 " + client.nickname + " :This server was created " + getMoment(_creationMoment));
    sendMsg(client.fd, ":ft_irc 004 " + client.nickname + " " + client.servername + " 1.0 uMode:none cMode:+i,+t,+k,+o,+l");
    sendMsg(client.fd, ":ft_irc 375 " + client.nickname + " :- Welcome to the Manicomio");
    sendMsg(client.fd, ":ft_irc 372 " + client.nickname + " :- Eat my Desktop Background");
    sendMsg(client.fd, ":ft_irc 376 " + client.nickname + " :- End of /MOTD command");
}

void Server::capCommand(const std::string &message, Client &client)
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
                    + (getClient(name).channels.size() > 0 ? " " + getClient(name).channels[0]->getName() + " " : " * ")
                    + getClient(name).getInfo()); 
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
    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " JOIN" + ERR_NEEDMOREPARAMS);
    else
    {
        std::vector<std::string> channelNames = split(getWord(message, 1), ',');
        std::vector<std::string> keys = split(getWord(message, 2), ',');

        for (size_t i = 0; i < channelNames.size(); i++)
        {
            std::string channelName = channelNames[i];
            if (channelName[0] != '#' && channelName[0] != '&')
                sendMsg(client.fd, PREFIX_ERR_NOSUCHCHANNEL + client.nickname + " " + channelName + ERR_NOSUCHCHANNEL);
            else if (channelExists(channelName) && _channels[channelName].getLimit()
                && _channels[channelName].getUsersSize() >= _channels[channelName].getLimit())
                sendMsg(client.fd, PREFIX_ERR_CHANNELISFULL + client.nickname + " " + channelName + ERR_CHANNELISFULL);
            else if (channelExists(channelName) && !_channels[channelName].isInvited(client.nickname))
                sendMsg(client.fd, PREFIX_ERR_INVITEONLYCHAN + client.nickname + " " + channelName + ERR_INVITEONLYCHAN);
            else if (channelExists(channelName) && ((keys.size() > i && !_channels[channelName].passMatch(keys[i]))
                        || (keys.size() <= i && !_channels[channelName].passMatch(""))))
                sendMsg(client.fd, PREFIX_ERR_BADCHANNELKEY + client.nickname + " " + channelName + ERR_BADCHANNELKEY);
            else
            {
                if(!channelExists(channelName))
                {
                    _channels[channelName] = Channel(this, channelName);
                    std::cout << "Channel " << channelName << " created" << std::endl;
                    client.joinChannel(&_channels[channelName], true);
                    sendMsg(client.fd, ":" + client.nickname + "!ft_irc" + "@" + _ip + " JOIN " + channelName);
                }
                else
                {
                    client.joinChannel(&_channels[channelName]);
                    _channels[channelName].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname + " JOIN " + channelName);
                    sendMsg(client.fd, ":ft_irc 324 " + client.nickname + " " + channelName + " " + _channels[channelName].getMode());
                    if (_channels[channelName].getTopic().empty())
                        sendMsg(client.fd, ":ft_irc 331 " + client.nickname + " " + channelName + " :No topic is set");
                    else
                        sendMsg(client.fd, ":ft_irc 332 " + client.nickname + " " + channelName + " :" + _channels[channelName].getTopic());
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
    else if (getWord(message, 1).find(':') == 0)
        sendMsg(client.fd, PREFIX_ERR_NORECIPIENT + client.nickname + " PRIVMSG" + ERR_NORECIPIENT);
    else if (message.find(':') == std::string::npos)
        sendMsg(client.fd, PREFIX_ERR_NOTEXTTOSEND + client.nickname + " PRIVMSG" + ERR_NOTEXTTOSEND);
    else
    {
        std::vector<std::string> targets = split(getWord(message, 1), ',');
        for (size_t i = 0; i < targets.size(); i++)
        {
            std::string target = targets[i];
            if (!clientExists(target) && !channelExists(target))
                sendMsg(client.fd, PREFIX_ERR_NOSUCHNICK + client.nickname + " " + target + ERR_NOSUCHNICK);
            else
            {
                std::string toSendMsg = ":" + client.nickname + "!" + client.username + "@" + client.hostname + " PRIVMSG "
                    + target + " " + message.substr(message.find(':'));
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

void Server::noticeCommand(const std::string &message, Client &client)
{
    if (!(countWords(message) < 2)
        && !(getWord(message, 1).find(':') == 0)
        && !(message.find(':') == std::string::npos))
    {
        std::vector<std::string> targets = split(getWord(message, 1), ',');
        for (size_t i = 0; i < targets.size(); i++)
        {
            std::string target = targets[i];
            if (!(!clientExists(target) && !channelExists(target)))
            {
                std::string toSendMsg = ":" + client.nickname + "!" + client.username + "@" + client.hostname + " NOTICE "
                    + target + " " + message.substr(message.find(':'));
                if (clientExists(target))
                    sendMsg(getClient(target).fd, toSendMsg);
                else
                {
                    if (!(_channels[target].userExists(client.nickname) == false))
                        _channels[target].sendMsg(toSendMsg, client.nickname);
                }
            }
        }
    }
}

void Server::modeCommand(const std::string &message, Client &client)
{
    std::string target = getWord(message, 1);
    std::string modes = getWord(message, 2);

    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " MODE" + ERR_NEEDMOREPARAMS);
    else if (!channelExists(target) && target.find_first_of("#&") == 0)
        sendMsg(client.fd, PREFIX_ERR_NOSUCHCHANNEL + client.nickname + " " + target + ERR_NOSUCHCHANNEL);
    else if (!clientExists(target) && !channelExists(target))
        sendMsg(client.fd, PREFIX_ERR_NOSUCHNICK + client.nickname + " " + target + ERR_NOSUCHNICK);
    else if (countWords(message) == 2)
    {
        if (channelExists(target))
        {
            sendMsg(client.fd, ":ft_irc 324 " + client.nickname + " " + target + " " + _channels[target].getMode());
            sendMsg(client.fd, ":ft_irc 329 " + client.nickname + " " + target + " " + _channels[target].getCreationMoment());
        }
        else
            sendMsg(client.fd, ":ft_irc 221 " + client.nickname + " :none");
    }
    else if (!_channels[target].userExists(client.nickname))
        sendMsg(client.fd, PREFIX_ERR_NOTONCHANNEL + client.nickname + " " + target + ERR_NOTONCHANNEL);
    else if (countWords(message) == 3 && modes.find_first_not_of("+-o") == std::string::npos
        && modes.find_first_of("+-") == 0 && modes.find('o') != std::string::npos)
        operatorMode(target, "", true, client);
    else if (!_channels[target].isOperator(client.nickname))
        sendMsg(client.fd, PREFIX_ERR_CHANOPRIVSNEEDED + client.nickname + " " + target + ERR_CHANOPRIVSNEEDED);
    else if (modes.find_first_of("+-") != 0)
        sendMsg(client.fd, PREFIX_ERR_CUSTOM + client.nickname + " MODE " + modes + " :Modestring must start with + or -");
    else if (modes.find_first_not_of("+-") == std::string::npos)
        sendMsg(client.fd, PREFIX_ERR_CUSTOM + client.nickname + " MODE " + modes + " :Modestring must have at least one mode");
    else
    {
        std::vector<std::string> modeArgs;
        std::istringstream iss(message);
        std::string word;
        iss >> word;
        iss >> word;
        iss >> word;
        while (iss >> word)
            modeArgs.push_back(word);

        std::string::iterator it = modes.begin();
        bool add = true;
        for (size_t i = 0; it != modes.end(); it++)
        {
            if (*it == '+' || *it == '-')
            {
                add = *it == '+';
                continue;
            }
            if (_modes.find(*it) == _modes.end())
                sendMsg(client.fd, PREFIX_ERR_UNKNOWNMODE + client.nickname + " " + *it + ERR_UNKNOWNMODE);
            else
            {
                std::string arg = i < modeArgs.size() ? modeArgs[i] : "";
                i += (this->*_modes[*it])(target, arg, add, client);
            }
        }
    }
}

bool Server::inviteOnlyMode(const std::string &channel, const std::string &argument, bool add, Client &client)
{
    (void)argument;

    _channels[channel].setInviteOnly(add);
    _channels[channel].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                                + " MODE " + channel + " " + (add ? "+i" : "-i"));

    return false;
}

bool Server::topicProtectedMode(const std::string &channel, const std::string &argument, bool add, Client &client)
{
    (void)argument;

    _channels[channel].setTopicProtected(add);
    _channels[channel].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                                + " MODE " + channel + " " + (add ? "+t" : "-t"));

    return false;
}

bool Server::passwordMode(const std::string &channel, const std::string &argument, bool add, Client &client)
{
    if (add)
    {
        if (argument.empty())
        {
            sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " MODE" + ERR_NEEDMOREPARAMS);
            return false;
        }
        if (!_channels[channel].passMatch(""))
        {
            sendMsg(client.fd, PREFIX_ERR_PASSWDMISMATCH + client.nickname + " MODE" + ERR_PASSWDMISMATCH);
            return true;
        }
        
        _channels[channel].setPassword(argument);
        _channels[channel].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                                    + " MODE " + channel + " +k ****");
        return true;
    }

    _channels[channel].setPassword("");
    _channels[channel].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                                + " MODE " + channel + " -k");
    return false;
}

bool Server::limitMode(const std::string &channel, const std::string &argument, bool add, Client &client)
{
    if (add)
    {
        if (argument.empty())
        {
            sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " MODE" + ERR_NEEDMOREPARAMS);
            return false;
        }
        if (argument.find_first_not_of("0123456789") != std::string::npos)
            sendMsg(client.fd, PREFIX_ERR_CUSTOM + client.nickname + " MODE " + channel + " :Limit must be a positive integer");
        if (strToNum<int>(argument) == 0)
            sendMsg(client.fd, PREFIX_ERR_CUSTOM + client.nickname + " MODE " + channel + " :Limit must be greater than 0");
        else
        {
            _channels[channel].setLimit(strToNum<int>(argument));
            _channels[channel].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                                        + " MODE " + channel + " +l " + argument);
        }
        return true;
    }

    _channels[channel].setLimit(0);
    _channels[channel].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                                + " MODE " + channel + " -l");
    return false;
}

bool Server::operatorMode(const std::string &channel, const std::string &argument, bool add, Client &client)
{
    if (argument.empty())
    {
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " MODE" + ERR_NEEDMOREPARAMS);
        return false;
    }

    if (!clientExists(argument))
        sendMsg(client.fd, PREFIX_ERR_NOSUCHNICK + client.nickname + " " + argument + ERR_NOSUCHNICK);
    else if (!_channels[channel].userExists(argument))
        sendMsg(client.fd, PREFIX_ERR_USERNOTINCHANNEL + client.nickname + " " + argument + " " + channel + ERR_USERNOTINCHANNEL);
    else
    {
        if (add)
            _channels[channel].unsetOperator(argument);
        else
            _channels[channel].setOperator(argument);
        _channels[channel].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                                    + " MODE " + channel + " " + (add ? "+o " : "-o ") + argument);
    }
    return true;
}

void Server::quitCommand(const std::string &message, Client &client)
{
    if (client.nickname.empty())
        std::cout << "Client " << client.fd << " disconnected" << std::endl;
    else
        std::cout << client.nickname << " disconnected" << std::endl;

    std::string reason;
    if (countWords(message) > 1 && message.find(':') != std::string::npos)
        reason = message.substr(message.find(':') + 1);
    else
        reason = "Reason not specified";

    for (std::vector<Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (it->nickname != client.nickname && client.isInAChannelWith(it->nickname))
            sendMsg(it->fd, ":" + client.nickname + "!" + client.username + "@" + client.hostname + " QUIT :" + reason);
    }
    removeClient(&client);
}

void Server::partCommand(const std::string &message, Client &client)
{
    if (countWords(message) < 2)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " PART" + ERR_NEEDMOREPARAMS);
    else
    {
        std::vector<std::string> channelNames = split(getWord(message, 1), ',');
        std::string reason;
        if (countWords(message) > 2 && message.find(':') != std::string::npos)
            reason = message.substr(message.find(':') + 1);
        else
            reason = "Reason not specified";
        
        for (size_t i = 0; i < channelNames.size(); i++)
        {
            std::string channelName = channelNames[i];
            if (!channelExists(channelName))
                sendMsg(client.fd, PREFIX_ERR_NOSUCHCHANNEL + client.nickname + " " + channelName + ERR_NOSUCHCHANNEL);
            else if (!_channels[channelName].userExists(client.nickname))
                sendMsg(client.fd, PREFIX_ERR_NOTONCHANNEL + client.nickname + " " + channelName + ERR_NOTONCHANNEL);
            else
            {
                _channels[channelName].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                                                + " PART " + channelName + " :" + reason);
                client.leaveChannel(&_channels[channelName]);
            }
        }
    }
}

void Server::kickCommand(const std::string &message, Client &client)
{
    std::string channelName = getWord(message, 1);

    if (countWords(message) < 3)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " KICK" + ERR_NEEDMOREPARAMS);
    else if (!channelExists(channelName))
        sendMsg(client.fd, PREFIX_ERR_NOSUCHCHANNEL + client.nickname + " " + channelName + ERR_NOSUCHCHANNEL);
    else if (!_channels[channelName].userExists(client.nickname))
        sendMsg(client.fd, PREFIX_ERR_NOTONCHANNEL + client.nickname + " " + channelName + ERR_NOTONCHANNEL);
    else if (!_channels[channelName].isOperator(client.nickname))
        sendMsg(client.fd, PREFIX_ERR_CHANOPRIVSNEEDED + client.nickname + " " + channelName + ERR_CHANOPRIVSNEEDED);
    else if (!clientExists(getWord(message, 2)))
        sendMsg(client.fd, PREFIX_ERR_NOSUCHNICK + client.nickname + " " + getWord(message, 2) + ERR_NOSUCHNICK);
    else if (!_channels[channelName].userExists(getWord(message, 2)))
        sendMsg(client.fd, PREFIX_ERR_USERNOTINCHANNEL + client.nickname
                + " " + getWord(message, 2) + " " + channelName + ERR_USERNOTINCHANNEL);
    else
    {
        std::string reason;
        if (countWords(message) > 3 && message.find(':') != std::string::npos)
            reason = message.substr(message.find(':') + 1);
        else
            reason = "Reason not specified";

        Client &target = getClient(getWord(message, 2));
        _channels[channelName].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                                        + " KICK " + channelName + " " + target.nickname + " :" + reason);
        target.leaveChannel(&_channels[channelName]);
    }
}

void Server::inviteCommand(const std::string &message, Client &client)
{
    std::string nickname = getWord(message, 1);
    std::string channelName = getWord(message, 2);

    if (countWords(message) < 3)
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " INVITE" + ERR_NEEDMOREPARAMS);
    else if (!clientExists(nickname))
        sendMsg(client.fd, PREFIX_ERR_NOSUCHNICK + client.nickname + " " + nickname + ERR_NOSUCHNICK);
    else if (!channelExists(channelName))
        sendMsg(client.fd, PREFIX_ERR_NOSUCHCHANNEL + client.nickname + " " + channelName + ERR_NOSUCHCHANNEL);
    else if (!_channels[channelName].userExists(client.nickname))
        sendMsg(client.fd, PREFIX_ERR_NOTONCHANNEL + client.nickname + " " + channelName + ERR_NOTONCHANNEL);
    else if (_channels[channelName].userExists(nickname))
        sendMsg(client.fd, PREFIX_ERR_USERONCHANNEL + client.nickname + " " + nickname + " " + channelName + ERR_USERONCHANNEL);
    else if(!_channels[channelName].isOperator(client.nickname) && _channels[channelName].isInviteOnly())
        sendMsg(client.fd, PREFIX_ERR_CHANOPRIVSNEEDED + client.nickname + " " + channelName + ERR_CHANOPRIVSNEEDED);
    else
    {
        _channels[channelName].addInvited(nickname);
        sendMsg(getClient(nickname).fd, ":" + client.nickname + "!" + client.username + "@" + client.hostname
                + " INVITE " + nickname + " " + channelName);
        sendMsg(client.fd, ":ft_irc 341 " + client.nickname + " " + nickname + " " + channelName);
    }
}

void Server::topicCommand(const std::string &message, Client &client)
{
    std::string channelName = getWord(message, 1);

    if (countWords(message) < 2 || (message.find(':') == std::string::npos && countWords(message) > 2))
        sendMsg(client.fd, PREFIX_ERR_NEEDMOREPARAMS + client.nickname + " TOPIC" + ERR_NEEDMOREPARAMS);
    else if (!channelExists(channelName))
        sendMsg(client.fd, PREFIX_ERR_NOSUCHCHANNEL + client.nickname + " " + channelName + ERR_NOSUCHCHANNEL);
    else if (!_channels[channelName].userExists(client.nickname))
        sendMsg(client.fd, PREFIX_ERR_NOTONCHANNEL + client.nickname + " " + channelName + ERR_NOTONCHANNEL);
    else if (countWords(message) == 2)
    {
        if (_channels[channelName].getTopic().empty())
            sendMsg(client.fd, ":ft_irc 331 " + client.nickname + " " + channelName + " :No topic is set");
        else
        {
            sendMsg(client.fd, ":ft_irc 332 " + client.nickname + " " + channelName + " :" + _channels[channelName].getTopic());
            sendMsg(client.fd, ":ft_irc 333 " + client.nickname + " " + channelName + " "
                        + _channels[channelName].getTopicAuthor() + " " + _channels[channelName].getTopicTime());
        }
    }
    else
    {
        if (!_channels[channelName].isOperator(client.nickname) && _channels[channelName].isTopicProtected())
            sendMsg(client.fd, PREFIX_ERR_CHANOPRIVSNEEDED + client.nickname + " " + channelName + ERR_CHANOPRIVSNEEDED);
        else
        {
            std::string topic = message.substr(message.find(':') + 1);
            _channels[channelName].setTopic(topic, client.nickname);
            _channels[channelName].sendMsg(":" + client.nickname + "!" + client.username + "@" + client.hostname
                        + " TOPIC " + channelName + " :" + topic);
        }
    }
}

void Server::handleMessage(std::string message, size_t iClient)
{
    if (iClient >= _clients.size())
    {
        throw ServerException("Error: invalid client index");
    }

    Client &client = _clients[iClient];

    std::vector<std::string> tokens = split(message, '\n');

    for (size_t i = 0; i < tokens.size(); i++)
    {
        std::string token = tokens[i];
        std::string command = getWord(token, 0);

        if (command == "CAP" || command == "PING" || (command == "PASS" && client.password.empty()))
            (this->*_commands[command])(token, client);
        else if (client.password.empty())
            sendMsg(client.fd, PREFIX_ERR_CUSTOM + std::string(":You must send a password first"));
        else if ((command == "NICK" && client.nickname.empty()) || (command == "USER" && client.username.empty()))
            (this->*_commands[command])(token, client);
        else if (client.nickname.empty())
            sendMsg(client.fd, PREFIX_ERR_CUSTOM + client.nickname + " :You must set a nickname first");
        else if (client.username.empty())
            sendMsg(client.fd, PREFIX_ERR_CUSTOM + client.nickname + " :You must set a username first");
        else if (command == "PASS" || command == "USER")
            sendMsg(client.fd, PREFIX_ERR_ALREADYREGISTRED + client.nickname + ERR_ALREADYREGISTRED);
        else if (_commands.find(command) != _commands.end())
            (this->*_commands[command])(token, client);
        else
            sendMsg(client.fd, PREFIX_ERR_UNKNOWNCOMMAND + client.nickname + " " + command + ERR_UNKNOWNCOMMAND);

        if (!client.authenticated && !client.password.empty() && !client.nickname.empty() && !client.username.empty())
        {
            client.authenticated = true;
            sendWelcomeMessage(client);
        }
        removeEmptyChannels();
    }

}

void Server::removeEmptyChannels()
{
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if (it->second.getUsersSize() == 0)
        {
            std::cout << "Channel " << it->first << " removed" << std::endl;
            _channels.erase(it);
            it = _channels.begin();
        }
    }
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
	_server_addr.sin_port = htons(_port);

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1)
        throw ServerException("Error getting hostname");

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, NULL, &hints, &res) != 0)
        throw ServerException("Error getting address info");
    
    _server_addr.sin_addr = reinterpret_cast<struct sockaddr_in *>(res->ai_addr)->sin_addr;
    freeaddrinfo(res);

	int opt = 1;
	setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(_fd, reinterpret_cast<struct sockaddr *>(&_server_addr), sizeof(_server_addr)) < 0)
	{
		close(_fd);
		throw ServerException("Error binding server");
	}

    _ip = inet_ntoa(_server_addr.sin_addr);
}

void Server::listenServer()
{
	if (listen(_fd, 10) < 0)
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

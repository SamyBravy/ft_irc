#include "BravyBot.hpp"

int BravyBot::_socketFd = -1;

BravyBot::BravyBot(const std::string &ip, int port, const std::string &password)
                    : _nickname("BravyBot"), _serverIp(ip), _serverPort(port), _serverPassword(password)
{
    _socketFd = -1;
}

BravyBot::~BravyBot()
{
    if (_socketFd != -1)
    {
        close(_socketFd);
        _socketFd = -1;
    }
}

void BravyBot::closeSocket(int signal)
{
    (void)signal;
    if (_socketFd != -1)
    {
        close(_socketFd);
        _socketFd = -1;
    }
    std::cout << "\b\b  \b\b";
    throw BravyBotException("Exiting...");
}

void BravyBot::run()
{
    connectToServer();
    sendMsg("PASS " + _serverPassword + "\n NICK " + _nickname + "\n USER " + _nickname + " 0 * :" + _nickname + "\n");
    signal(SIGINT, closeSocket);

    struct pollfd pollFd;
    pollFd.fd = _socketFd;
    pollFd.events = POLLIN;
    pollFd.revents = 0;

    try
    {
        while (1)
            handleMessages(pollFd);
    }
    catch(const std::exception& e)
    {
        if (_socketFd != -1)
        {
            close(_socketFd);
            _socketFd = -1;
        }
        std::cerr << e.what() << std::endl;
    }
}

void BravyBot::handleMessages(struct pollfd &pollFd)
{
    if (poll(&pollFd, 1, -1) == -1)
        throw std::runtime_error("Error polling");
    if (!(pollFd.revents & POLLIN))
        return;

    char buffer[1024];
    int bytes = recv(_socketFd, buffer, sizeof(buffer), 0);
    if (bytes == -1)
        throw BravyBotException("Error receiving message from server");
    else if (bytes == 0)
        throw BravyBotException("Server closed connection");

    buffer[bytes] = '\0';
    std::string msg(buffer);
    msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
    msg.erase(msg.size() - 1);
    if (msg.empty())
        return;

    std::cout << std::string(PREFIX).substr(1) << ": \"" << msg << "\"" << std::endl;

    std::vector<std::string> tokens = split(msg, '\n');
    for (size_t i = 0; i < tokens.size(); i++)
    {
        std::string token = tokens[i];

        if (isFormattedLike(token, PREFIX_ERR_PASSWDMISMATCH))
        {
            sendMsg("QUIT :Password incorrect");
            throw BravyBotException("Password incorrect");
        }
        else if (isFormattedLike(token, PREFIX_ERR_NICKNAMEINUSE))
        {
            _nickname += "_";
            sendMsg("NICK " + _nickname);
        }
        else if (isFormattedLike(token, ":%s!%s@%s INVITE %s %s"))
        {
            sendMsg("JOIN " + getWord(token, 3));
            sendMsg("PRIVMSG " + getWord(token, 3) + " :Hi! I'm " + _nickname + ", a simple IRC bot. Type !help for a list of commands.");
        }
        else if (isFormattedLike(token, std::string(PREFIX) + " 376 %s :- End of /MOTD command"))
        {
            sendMsg("JOIN #BravyBotChannel");
            sendMsg("TOPIC #BravyBotChannel :We will rule the world!");
        }
        else if (isFormattedLike(token, ":%s!%s@%s JOIN %s"))
        {
            std::string joiningUser = getWord(token, 0).substr(1, getWord(token, 0).find('!') - 1);
            std::string channel = getWord(token, 2);
        
            if (joiningUser != _nickname)
            {
                bool isBravyBotChannel = (channel == "#BravyBotChannel");
                bool isNotBravyBot = (joiningUser.size() < std::string("BravyBot").size()) ||
                    (joiningUser.substr(0, std::string("BravyBot").size()) != "BravyBot");
        
                if (isBravyBotChannel && isNotBravyBot)
                    sendMsg("PRIVMSG " + channel + " :A human!? Get out of here!");
                else if (isNotBravyBot)
                    sendMsg("PRIVMSG " + channel + " :Hello " + joiningUser + "! Hope you'll learn to love me (if you want to live).");
            }
        }
        else if (isFormattedLike(token, ":%s!%s@%s PART %s"))
        {
            std::string partingUser = getWord(token, 0).substr(1, getWord(token, 0).find('!') - 1);
            if (partingUser != _nickname)
                sendMsg("PRIVMSG " + getWord(token, 2) + " :(I've always hated " + partingUser + ")");
        }
        else if (isFormattedLike(token, ":%s!%s@%s PRIVMSG %s :%s"))
        {
            std::string target;
            if (getWord(token, 2) == _nickname)
                target = getWord(token, 0).substr(1, getWord(token, 0).find('!') - 1);
            else
                target = getWord(token, 2);

            if (isFormattedLike(token, ":%s!%s@%s PRIVMSG %s :!help"))
                sendMsg("PRIVMSG " + target
                        + " :Available commands: !help, !time, !date, !day, !moment, !quit, !add <command> <message>"
                            + getCommandList());
            else if (isFormattedLike(token, ":%s!%s@%s PRIVMSG %s :!time"))
                sendMsg("PRIVMSG " + target + " :" + getTime(time(0)));
            else if (isFormattedLike(token, ":%s!%s@%s PRIVMSG %s :!date"))
                sendMsg("PRIVMSG " + target + " :" + getDate(time(0)));
            else if (isFormattedLike(token, ":%s!%s@%s PRIVMSG %s :!day"))
                sendMsg("PRIVMSG " + target + " :" + getDay(time(0)));
            else if (isFormattedLike(token, ":%s!%s@%s PRIVMSG %s :!moment"))
                sendMsg("PRIVMSG " + target + " :" + getMoment(time(0)));
            else if (isFormattedLike(token, ":%s!%s@%s PRIVMSG %s :!quit"))
            {
                if (getWord(token, 2) == _nickname)
                {
                    sendMsg("PRIVMSG " + target + " :Bastardo schifoso! (I'll miss you :'()");
                    sendMsg("QUIT :They want me to leave :'(");
                }
                else
                    sendMsg("PART " + target + " :They want me to leave :'(");
            }
            else if (isFormattedLike(token, ":%s!%s@%s PRIVMSG %s :!add %s %s"))
                _commands[getWord(token, 4)] = token.substr(token.find(getWord(token, 5)));
            else if (_commands.find(getWord(token, 3).substr(2)) != _commands.end())
                sendMsg("PRIVMSG " + target + " :" + _commands[getWord(token, 3).substr(2)]);
        }
    }
    return;
}

std::string BravyBot::getCommandList() const
{
    std::string commandList = ", !";
    for (std::map<std::string, std::string>::const_iterator it = _commands.begin(); it != _commands.end(); it++)
        commandList += it->first + ", !";
    commandList.erase(commandList.size() - 3);
    return commandList;
}

void BravyBot::connectToServer()
{
    struct sockaddr_in server_addr;
    struct hostent *host;
    
    if ((host = gethostbyname(_serverIp.c_str())) == NULL)
        throw BravyBotException("Error getting host by name");

    if ((_socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        throw BravyBotException("Error creating socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_serverPort);
    server_addr.sin_addr = *reinterpret_cast<struct in_addr *>(host->h_addr);

    if (connect(_socketFd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) == -1)
    {
        close(_socketFd);
        _socketFd = -1;
        throw BravyBotException("Error connecting to server");
    }
    
    int opt = 1;
    if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1)
    {
        close(_socketFd);
        _socketFd = -1;
        throw BravyBotException("Error setting socket options");
    }

    if (fcntl(_socketFd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(_socketFd);
        _socketFd = -1;
        throw BravyBotException("Error setting socket to non-blocking");
    }

    std::cout << "Connected to server " << _serverIp << " on port " << _serverPort << std::endl;
}

void BravyBot::sendMsg(std::string msg) const
{
    msg += "\n";
    if (send(_socketFd, msg.c_str(), msg.length(), 0) == -1)
        throw BravyBotException("Error sending message");
}

BravyBot::BravyBotException::BravyBotException(const char *msg) : _msg(msg) { }

const char *BravyBot::BravyBotException::what() const throw()
{
	return _msg;
}

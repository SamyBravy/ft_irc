#include "CaccaBot.hpp"

CaccaBot::CaccaBot(const std::string &ip, int port, const std::string &password)
                    : _nickname("CaccaBot"), _socketFd(-1), _serverIp(ip), _serverPort(port), _serverPassword(password) { }

CaccaBot::~CaccaBot()
{
    if (_socketFd != -1)
        close(_socketFd);
}

void CaccaBot::run()
{
    connectToServer();
    sendMsg("PASS " + _serverPassword + "\n NICK " + _nickname + "\n USER " + _nickname + " 0 * :" + _nickname);

    while (1)
    {
        char buffer[1024];
        int bytes = recv(_socketFd, buffer, sizeof(buffer), 0);
        if (bytes == -1)
            throw CaccaBotException("Error receiving message from server");
        else if (bytes == 0)
        {
            std::cout << "Server closed connection" << std::endl;
            break;
        }

        buffer[bytes] = '\0';
        std::string msg(buffer);
        msg.erase(std::remove(msg.begin(), msg.end(), '\r'), msg.end());
        msg.erase(msg.size() - 1);
        if (msg.empty())
            continue;

        if (_serverName.empty())
            _serverName = getWord(msg, 0).substr(1);

        std::cout << _serverName << ": \"" << msg << "\"" << std::endl;

        std::vector<std::string> tokens = split(msg, '\n');
        for (size_t i = 0; i < tokens.size(); i++)
        {
            std::string token = tokens[i];

            const std::string errPrefix = PREFIX_ERR_NICKNAMEINUSE;
            if (token.size() >= errPrefix.size() && token.substr(0, errPrefix.size()) == errPrefix)
            {
                _nickname += "_";
                sendMsg("NICK " + _nickname);
            }
            else if (token.find(_serverName) != 1 && token.find('!') != std::string::npos && token.find('@') != std::string::npos
                        && token.find(" INVITE ") != std::string::npos)
                sendMsg("JOIN " + getWord(token, 3));
        }
    }
}

void CaccaBot::connectToServer()
{
    struct sockaddr_in server_addr;
    struct hostent *host;
    
    if ((host = gethostbyname(_serverIp.c_str())) == NULL)
        throw CaccaBotException("Error getting host by name");

    if ((_socketFd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        throw CaccaBotException("Error creating socket");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_serverPort);
    server_addr.sin_addr = *reinterpret_cast<struct in_addr *>(host->h_addr);

    if (connect(_socketFd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) == -1)
    {
        close(_socketFd);
        throw CaccaBotException("Error connecting to server");
    }

    std::cout << "Connected to server " << _serverIp << " on port " << _serverPort << std::endl;
}

void CaccaBot::sendMsg(std::string msg) const
{
    msg += "\n";
    if (send(_socketFd, msg.c_str(), msg.length(), 0) == -1)
        throw CaccaBotException("Error sending message");
}

CaccaBot::CaccaBotException::CaccaBotException(const char *msg) : _msg(msg) { }

const char *CaccaBot::CaccaBotException::what() const throw()
{
	return _msg;
}

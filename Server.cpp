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

struct pollfd *Server::getPollfds()
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
        
    if (_clients[i].fd != -1)
    {
        close(_clients[i].fd);
    }
	_clients.erase(_clients.begin() + i);
}

void Server::handleMessage(std::string buffer, int iClient)
{
    if (iClient < 0 || iClient >= (int)_clients.size())
    {
        throw ServerException("Error: invalid client index");
    }

    if (!buffer.empty() && buffer[0] == ':' && buffer.find(' ') != std::string::npos)
    {
        buffer = buffer.substr(buffer.find(' ') + 1);
    }

	std::vector<std::string> tokens = split(buffer, '\n');

    for (size_t i = 0; i < tokens.size(); i++)
    {
        if (isStartSubstring(tokens[i], "CAP"))
            continue;
        else if (isStartSubstring(tokens[i], "PASS"))
        {
            if (countWords(tokens[i]) < 2)
                sendMsg(_clients[iClient].fd, PREFIX_ERR_NEEDMOREPARAMS + _clients[iClient].nickname + " PASS" + ERR_NEEDMOREPARAMS);
            else if (_clients[iClient].authenticated == true)
                sendMsg(_clients[iClient].fd, PREFIX_ERR_ALREADYREGISTRED + _clients[iClient].nickname + ERR_ALREADYREGISTRED);
            else
            {
                _clients[iClient].password = getWord(tokens[i], 1);
                _clients[iClient].authState |= 0b00000001;
            }
        }
        else if (_clients[iClient].authState & 0b00000001)
        {
            if (isStartSubstring(tokens[i], "NICK"))
            {
                if (countWords(tokens[i]) < 2)
                    sendMsg(_clients[iClient].fd, PREFIX_ERR_NONICKNAMEGIVEN + _clients[iClient].nickname + ERR_NONICKNAMEGIVEN);
                else if (clientExists(getWord(tokens[i], 1)))
                    sendMsg(_clients[iClient].fd, PREFIX_ERR_NICKNAMEINUSE + _clients[iClient].nickname + " " + getWord(tokens[i], 1) + ERR_NICKNAMEINUSE);
                else
                {
                    _clients[iClient].nickname = getWord(tokens[i], 1);
                    _clients[iClient].authState |= 0b00000010;
                }
            }
            else if (isStartSubstring(tokens[i], "USER"))
            {
                if (countWords(tokens[i]) < 5)
                    sendMsg(_clients[iClient].fd, PREFIX_ERR_NEEDMOREPARAMS + _clients[iClient].nickname + " USER" + ERR_NEEDMOREPARAMS);
                else if (_clients[iClient].authenticated == true)
                    sendMsg(_clients[iClient].fd, PREFIX_ERR_ALREADYREGISTRED + _clients[iClient].nickname + ERR_ALREADYREGISTRED);
                else
                {
                    _clients[iClient].username = getWord(tokens[i], 1);
                    _clients[iClient].hostname = getWord(tokens[i], 2);
                    _clients[iClient].servername = getWord(tokens[i], 3);
                    _clients[iClient].realname = tokens[i].substr(tokens[i].find(':', 1) + 1);
                    _clients[iClient].authState |= 0b00000100;
                }
            }
        }
        else if (_clients[iClient].authenticated == true)
        {

        }

        if (!_clients[iClient].authenticated && _clients[iClient].authState == 0b00000111)
        {
            if (_clients[iClient].password == _password)
            {
                _clients[iClient].authenticated = true;
                sendMsg(_clients[iClient].fd, ":ft_irc 001 " + _clients[iClient].nickname + " :Welcome to the ft_irc network, " + _clients[iClient].nickname + "!" + _clients[iClient].username + "@" + _clients[iClient].hostname);
            }
            else
                sendMsg(_clients[iClient].fd, PREFIX_ERR_PASSWDMISMATCH + _clients[iClient].nickname + ERR_PASSWDMISMATCH);
        }
    }
}

void Server::createSocket()
{
	_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_fd < 0)
		throw ServerException("Error creating socket");
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

bool Server::clientExists(const std::string &nickname)
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i].nickname == nickname)
            return true;
    }
    return false;
}

Server::ServerException::ServerException(const char *msg) : _msg(msg) { }

const char *Server::ServerException::what() const throw()
{
	return _msg;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdell-er <sdell-er@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 12:11:07 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/19 12:11:08 by sdell-er         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port, std::string password) : _fd(-1), _port(port), _password(password) { }

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
	_clients.push_back(Client(client_fd));
}

void Server::removeClient(int i)
{
	_clients.erase(_clients.begin() + i);
}

void Server::handleMessage(std::string buffer, int i)
{
	if (_clients[i].authenticated == false)
	{
		std::vector<std::string> tokens = split(buffer, "\r\n");
		
		for (size_t i = 0; i < tokens.size(); i++)
		{
			if (tokens[i].find("CAP LS ") != std::string::npos)
			{
				continue;
			}
			else if (tokens[i].find("PASS ") != std::string::npos)
			{
				if (split(tokens[i], " ").size() >= 2 && split(tokens[i], " ")[1] == _password)
				{
					_clients[i].authenticated = true;
					std::cout << "Client " << _clients[i].fd << " authenticated" << std::endl;
				}
				else
				{
					std::cout << "Client " << _clients[i].fd << " failed to authenticate" << std::endl;
					removeClient(i);
					return;
				}
				continue;
			}
		}
		
		if (_clients[i].authenticated == false)
		{
			std::cout << "Client " << _clients[i].fd << " failed to authenticate" << std::endl;
			removeClient(i);
			return;
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
}

void Server::listenServer()
{
	if (listen(_fd, MAX_CLIENTS) < 0)
	{
		close(_fd);
		throw ServerException("Error listening server");
	}

	std::cout << "Server listening on port " << _port << std::endl;
}

Server::ServerException::ServerException(const char *msg) : _msg(msg) { }

const char *Server::ServerException::what() const throw()
{
	return _msg;
}

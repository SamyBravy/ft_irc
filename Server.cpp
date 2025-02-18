/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sdell-er <sdell-er@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/18 15:26:58 by sdell-er          #+#    #+#             */
/*   Updated: 2025/02/18 16:50:01 by sdell-er         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port) : _fd(-1), _port(port) { }

Server::~Server()
{
	if (_fd != -1)
		close(_fd);
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

	for (size_t i = 1; i < _clients.size(); i++)
	{
		fds[i].fd = _clients[i]._fd;
		fds[i].events = POLLIN;
	}

	for (size_t i = _clients.size(); i <= MAX_CLIENTS; i++)
		fds[i].fd = -1;

	return fds;
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

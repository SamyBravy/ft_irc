/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samuele <samuele@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 22:02:33 by samuele           #+#    #+#             */
/*   Updated: 2025/02/17 23:27:28 by samuele          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Server::Server(int port) : _fd(-1), _port(port), _channelsCount(0) { }

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

void Server::createSocket()
{
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_fd < 0)
        throw ServerException("Error creating socket");

    std::cout << "Server created" << std::endl;
}

void Server::bindServer()
{
    struct sockaddr_in _server_addr;

    memset(&_server_addr, 0, sizeof(_server_addr));
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_addr.s_addr = INADDR_ANY;
    _server_addr.sin_port = htons(_port);

    if (bind(_fd, (struct sockaddr *)&_server_addr, sizeof(_server_addr)) < 0)
    {
        close(_fd);
        throw ServerException("Error binding server");
    }

    std::cout << "Server binded" << std::endl;
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

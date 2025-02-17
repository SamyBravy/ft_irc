/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samuele <samuele@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 23:18:32 by samuele           #+#    #+#             */
/*   Updated: 2025/02/17 23:31:53 by samuele          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_irc.hpp"
#include "Server.hpp"
#include "Channel.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Error: usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    if (!isDigit(argv[1]))
    {
        std::cerr << "Error: port must be a natural number" << std::endl;
        return 1;
    }

    Server server(strToNum<int>(argv[1]));

    try
    {
        server.run();
    }
    catch (const Server::ServerException &e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    
    // Inizializziamo l'array di file descriptor per poll()
    struct pollfd fds[MAX_CLIENTS + 1];  // +1 per il socket del server
    fds[0].fd = server.getFd();      // Il primo fd è il socket del server
    fds[0].events = POLLIN;  // Monitoriamo nuove connessioni

    for (int i = 1; i <= MAX_CLIENTS; i++)
        fds[i].fd = -1;  // Inizializziamo gli slot per i client

    while (1)
    {
        int ret = poll(fds, MAX_CLIENTS + 1, -1); // Attende eventi
        if (ret > 0)
        {
            // 1. Controlliamo se ci sono nuove connessioni
            if (fds[0].revents & POLLIN)
            {
                int client_fd = accept(server.getFd(), NULL, NULL);
                if (client_fd != -1)
                {
                    for (int i = 1; i <= MAX_CLIENTS; i++)
                    {
                        if (fds[i].fd == -1)
                        { // Troviamo uno slot libero
                            fds[i].fd = client_fd;
                            fds[i].events = POLLIN;  // Monitoriamo i dati in arrivo
                            std::cout << "New client connected (fd: " << client_fd << ")" << std::endl;
                            break;
                        }
                    }
                }
            }

            // 2. Controlliamo se qualche client ha inviato dati
            for (int i = 1; i <= MAX_CLIENTS; i++)
            {
                if (fds[i].fd != -1 && (fds[i].revents & POLLIN))
                {
                    char buffer[1024];
                    ssize_t n = read(fds[i].fd, buffer, sizeof(buffer));
                    if (n > 0)
                    {
                        buffer[n] = '\0';
                        std::cout << "Client " << fds[i].fd << " says: \"" << buffer << "\"" << std::endl;
                    }
                    else
                    {
                        std::cout << "Client " << fds[i].fd << " disconnected" << std::endl;
                        close(fds[i].fd);
                        fds[i].fd = -1;
                    }
                }
            }
        }
        else if (ret == -1)
            return 1;
    }

    return 0;
}

bool isDigit(std::string str)
{
    for (size_t i = 0; i < str.length(); i++)
    {
        if (!isdigit(str[i]))
            return false;
    }
    return true;
}


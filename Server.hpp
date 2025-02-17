/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samuele <samuele@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 21:52:29 by samuele           #+#    #+#             */
/*   Updated: 2025/02/17 23:27:43 by samuele          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include "Channel.hpp"
# include "ft_irc.hpp"

class Server
{    
    private:
        int _fd;
        int _port;
        Channel _channels[MAX_CHANNELS];
        int _channelsCount;
        
        void createSocket();
        void bindServer();
        void listenServer();
        
    public:
        Server(int port = 8080);
        ~Server();

        int getFd() const { return _fd; }

        void run();

        class ServerException : public std::exception
        {
            private:
                const char *_msg;
            
            public:
                ServerException(const char *msg);
                virtual const char *what() const throw();
        };
};

#endif

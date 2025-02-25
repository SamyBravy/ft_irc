#ifndef CACCABOT_HPP
# define CACCABOT_HPP

# include "../ft_irc.hpp"

class CaccaBot
{
    private:
        std::string _nickname;
        int _serverFd;
        std::string _serverIp;
        int _serverPort;
        std::string _serverPassword;
        std::string _serverName;

        void connectToServer();
        void sendMsg(std::string msg) const;
    
    public:
        CaccaBot(const std::string &ip = "127.0.1.1", int port = 8080, const std::string &password = "password");
        ~CaccaBot();

        void run();

        class CaccaBotException : public std::exception
        {
            private:
                const char *_msg;
            
            public:
                CaccaBotException(const char *msg);
                virtual const char *what() const throw();
        };
};

#endif

#ifndef CACCABOT_HPP
# define CACCABOT_HPP

# include "../ft_irc.hpp"

class CaccaBot
{
    private:
        std::string _nickname;
        static int _socketFd;
        std::string _serverIp;
        int _serverPort;
        std::string _serverPassword;
        std::map<std::string, std::string> _commands;

        void connectToServer();
        void handleMessages(struct pollfd &pollFd);
        static void closeSocket(int signal);
        void sendMsg(std::string msg) const;
    
    public:
        CaccaBot(const std::string &ip = "127.0.1.1", int port = 8080, const std::string &password = "password");
        ~CaccaBot();

        void run();

        std::string getCommandList() const;

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

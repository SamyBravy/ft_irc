# https://chatgpt.com/c/67afec25-ec14-800c-8ff0-a859501182d0
# https://chatgpt.com/c/67b37fb3-a6f4-800c-ac29-1848245ad517
# https://www.alien.net.au/irc/irc2numerics.html
# https://dd.ircdocs.horse/refs/commands/motd
# https://modern.ircdocs.horse/

NAME = ircserv
SRC = main.cpp \
	Server.cpp \
	Client.cpp \
	Channel.cpp

FLAGS = -Wall -Wextra -Werror -std=c++98 \
-pedantic -Wshadow -Wfloat-equal \
			 -Wundef -Wredundant-decls -Wold-style-cast -Wnon-virtual-dtor \
			-Woverloaded-virtual -Wformat=2 \
			-std=c++98 -g -Ofast
CC = c++

all: $(NAME)

$(NAME): $(SRC)
	@$(CC) $(SRC) -o $(NAME) $(FLAGS)
	@echo "\e[0;93m[$(NAME)] compiled!\e[0m"

clean:
	@echo > /dev/null

fclean:
	@rm -f $(NAME)
	@echo "\e[0;91m[$(NAME)] deleted!\e[0m"
	@rm -f clientMessages.txt
	@rm -f serverMessages.txt

re: fclean all

.PHONY: all clean fclean re

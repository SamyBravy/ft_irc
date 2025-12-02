# https://chatgpt.com/c/67afec25-ec14-800c-8ff0-a859501182d0
# https://chatgpt.com/c/67b37fb3-a6f4-800c-ac29-1848245ad517
# https://www.alien.net.au/irc/irc2numerics.html
# https://dd.ircdocs.horse/refs/commands/motd
# https://modern.ircdocs.horse/

NAME_S = ircserv
NAME_B = ircbot

SRC_S = main.cpp \
		./Server/Server.cpp \
		./Server/Client.cpp \
		./Server/Channel.cpp
SRC_B = main.cpp \
		./Bot/BravyBot.cpp

FLAGS = -Wall -Wextra -Werror -std=c++98 \
		-pedantic -Wshadow -Wfloat-equal \
		-Wundef -Wredundant-decls -Wold-style-cast -Wnon-virtual-dtor \
		-Woverloaded-virtual -Wformat=2 \
		-g -Ofast

CC = c++

all: $(NAME_S) $(NAME_B)

$(NAME_S): $(SRC_S)
	@$(CC) -D IS_SERVER=1 $(SRC_S) -o $(NAME_S) $(FLAGS)
	@echo "\e[0;93m[$(NAME_S)] compiled!\e[0m"
$(NAME_B): $(SRC_B)
	@$(CC) -D IS_SERVER=0 $(SRC_B) -o $(NAME_B) $(FLAGS)
	@echo "\e[0;93m[$(NAME_B)] compiled!\e[0m"

clean:
	@echo > /dev/null

fclean:
	@rm -f $(NAME_S) $(NAME_B)
	@echo "\e[0;91m[$(NAME_S)] deleted!\e[0m"
	@rm -f clientMessages.txt
	@rm -f serverMessages.txt

re: fclean all

.PHONY: all clean fclean re

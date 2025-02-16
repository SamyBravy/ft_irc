# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sdell-er <sdell-er@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/11/24 23:26:33 by sdell-er          #+#    #+#              #
#    Updated: 2025/02/16 15:49:30 by sdell-er         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

# https://www.alien.net.au/irc/irc2numerics.html

NAME = ircserv
SRC = main.cpp

FLAGS = -Wall -Wextra -Werror -std=c++98
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

re: fclean all

.PHONY: all clean fclean re

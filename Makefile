# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hmrabet <hmrabet@student.1337.ma>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/20 11:17:19 by hmrabet           #+#    #+#              #
#    Updated: 2025/03/15 08:09:16 by hmrabet          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

SOURCE = src/main.cpp \
		src/Request/Request.cpp src/Request/parser.cpp src/Request/parser-request_line.cpp \
		src/Request/parser-headers.cpp src/Request/parser-body.cpp src/Request/parser-multipart.cpp \
		src/Request/utils.cpp src/Multipart.cpp \
		# src/response.cpp src/server.cpp src/cgi.cpp src/utils.cpp

OBJECT = $(SOURCE:.cpp=.o)

HEADERS = includes/webserv.hpp includes/Request.hpp includes/Multipart.hpp # includes/server.hpp  includes/response.hpp includes/cgi.hpp

INCLUDES = -Iincludes

OBJ_DIR = obj

CPP = @c++ -Wall -Wextra -Werror -std=c++98

GREEN = \033[32m
RESET = \033[0m

define PRINT_LOADING
	@printf "$(GREEN)Compiling... ["
	@for i in $(shell seq 0 10 100); do 		printf "▓"; 		sleep 0.1; 	done
	@printf "] 100%%$(RESET)\n"
endef

all : $(NAME)

$(NAME) : $(OBJECT)
	$(CPP) $^ -o $(NAME)
	@echo "$(GREEN)Your program is ready!$(RESET)"

%.o : %.cpp Makefile $(HEADERS)
	@echo "$(GREEN) $<"
	@$(PRINT_LOADING)
	$(CPP) $(INCLUDES) -c $< -o $@

clean :
	@rm -f $(OBJECT)

fclean : clean
	@rm -f $(NAME)

re : fclean all

.PHONY : clean

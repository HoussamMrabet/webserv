# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mel-hamd <mel-hamd@student.1337.ma>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/20 11:17:19 by hmrabet           #+#    #+#              #
#    Updated: 2025/03/21 10:51:32 by mel-hamd         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv

SOURCE = src/main.cpp src/TokenizeFile.cpp # src/requests.cpp src/response.cpp src/server.cpp src/cgi.cpp src/utils.cpp

OBJECT = $(SOURCE:.cpp=.o)

HEADERS = includes/webserv.hpp includes/TokenizeFile.hpp  #includes/Response.hpp includes/methods/Delete.hpp includes/methods/Get.hpp includes/methods/Post.hpp includes/methods/Put.hpp  # includes/server.hpp includes/request.hpp includes/response.hpp includes/cgi.hpp  

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

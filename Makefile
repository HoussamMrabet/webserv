# This makefile uses wildcards to facilitate testing the code
# the old one is still present and called Makefile_old
# you can remove this one and use the old Makefile

NAME      =		webserv
CPP       =		c++
CPPFLAGS  =		-std=c++98 -Iincludes -Wall -Wextra -Werror
SRC_DIR   =		src
OBJ_DIR   =		obj
INC_DIR   =		includes
SRCS      =		$(shell find $(SRC_DIR) -name "*.cpp") # src/ and subdirs
OBJS      =		$(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS)) # replace src/*.cpp with obj/*.o

RM        =		rm -rf

GREEN 	= \033[32m
MAGENTA = \033[35m
RESET 	= \033[0m

define PRINT_LOADING
	@for i in $(shell seq 0 10 100); do 		printf "▓"; 		sleep 0; 	done
	@printf "] 100%%$(RESET)\n"
endef

all: $(OBJ_DIR) $(NAME)

$(NAME): $(OBJS)
	@$(CPP) $(CPPFLAGS) $^ -o $@
	@echo "$(GREEN)Your program is ready!$(RESET)"

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(INC_DIR)/*.hpp
	@mkdir -p $(dir $@) 
	@echo "$(GREEN) $<"
	@printf "$(GREEN)Compiling... ["
	@$(PRINT_LOADING)
	@$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	@printf "$(MAGENTA)Cleaning object files... ["
	@$(PRINT_LOADING)
	@$(RM) $(OBJ_DIR)

fclean: clean
	@printf "$(MAGENTA)Removing binary...["
	@$(PRINT_LOADING)
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

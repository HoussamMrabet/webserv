NAME      =		webserv
CPP       =		c++
CPPFLAGS  =		-std=c++98 -Iincludes -Wall -Wextra -Werror
SRC_DIR   =		src
OBJ_DIR   =		obj
INC_DIR   =		includes
SRCS      =		src/cgi.cpp src/ConfigBuilder.cpp src/LocationConf.cpp src/main.cpp src/Multipart.cpp \
				src/Request/parser-body.cpp src/Request/parser-headers.cpp src/Request/parser-multipart.cpp \
				src/Request/parser-request_line.cpp src/Request/parser-request_processing.cpp \
				src/Request/parser.cpp src/Request/Request.cpp src/Request/utils.cpp src/response/DeleteResponse.cpp \
				src/response/DirectoryListing.cpp src/response/ErrorResponse.cpp src/response/GetResponse.cpp \
				src/response/MimeTypes.cpp src/response/PostResponse.cpp src/response/RedirectResponse.cpp \
				src/response/response.cpp src/server/Connection.cpp src/server/ReadRequest.cpp src/server/SendResponse.cpp \
				src/server/Socket.cpp src/server/WebServer.cpp src/ServerConf.cpp src/TokenizeFile.cpp

OBJS      =		$(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

RM        =		rm -rf

GREEN 	= \033[32m
MAGENTA = \033[35m
RESET 	= \033[0m

define PRINT_LOADING
	@printf "$(GREEN)Compiling... ["
	@for i in $(shell seq 0 10 100); do 		printf "▓"; 		sleep 0.01; 	done
	@printf "] 100%%$(RESET)\n"
endef

define PRINT_LOADING
	@printf "$(GREEN)Compiling... ["
	@for i in $(shell seq 0 10 100); do 		printf "▓"; 		sleep 0.01; 	done
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
	@$(PRINT_LOADING)
	@$(CPP) $(CPPFLAGS) -c $< -o $@

clean:
	@printf "$(MAGENTA)Cleaning object files... ["
	@for i in $(shell seq 0 10 100); do 		printf "▓"; 		sleep 0.05; 	done
	@printf "] 100%%$(RESET)\n"
	@$(RM) $(OBJ_DIR)

fclean: clean
	@printf "$(MAGENTA)Removing executable file...["
	@for i in $(shell seq 0 10 100); do 		printf "▓"; 		sleep 0.01; 	done
	@printf "] 100%%$(RESET)\n"
	@$(RM) $(NAME)


re: fclean all

.PHONY: all clean fclean re

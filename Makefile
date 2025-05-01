# Compiler and flags
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Project name
NAME = ircserv

# Directories
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Headers
HEADERS = $(wildcard $(INC_DIR)/*.hpp)

# Server settings
PORT = 6667
PASSWORD = password
DEFAULT_NICK = testuser

# Colors for terminal output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
MAGENTA = \033[0;35m
CYAN = \033[0;36m
RESET = \033[0m

# Rules
all: $(NAME)

$(NAME): $(OBJS)
	@echo "$(BLUE)Linking $(NAME)...$(RESET)"
	@$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "$(GREEN)$(NAME) successfully compiled!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) Makefile | $(OBJ_DIR)
	@echo "$(YELLOW)Compiling $<...$(RESET)"
	@$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@

$(OBJ_DIR):
	@echo "$(MAGENTA)Creating object directory...$(RESET)"
	@mkdir -p $(OBJ_DIR)

clean:
	@echo "$(RED)Removing object files...$(RESET)"
	@rm -rf $(OBJ_DIR)

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(RESET)"
	@rm -f $(NAME)

re: fclean all

debug: CXXFLAGS += -g
debug: re

run: all
	./$(NAME) $(PORT) $(PASSWORD)

nc:
	@echo "$(CYAN)Connecting to server with nc...$(RESET)"
	@echo "$(YELLOW)Type these commands to register:$(RESET)"
	@echo "  PASS $(PASSWORD)"
	@echo "  NICK $(DEFAULT_NICK)"
	@echo "  USER $(DEFAULT_NICK) 0 * :Test User"
	@nc -C localhost $(PORT)

irssi:
	@echo "$(CYAN)Connecting to server with irssi...$(RESET)"
	@irssi -c localhost -p $(PORT) -n $(DEFAULT_NICK) -w $(PASSWORD)

test: all
	@echo "$(CYAN)Running tests...$(RESET)"
	@echo "To connect with netcat, use: nc -C localhost <port>"
	@echo "To connect with irssi, use: irssi -c localhost -p <port> -n nickname -w password"

git:
	@make fclean
	@echo "$(CYAN)Adding all changes to git...$(RESET)"
	@git add .
	@echo "$(CYAN)Committing changes...$(RESET)"
	@git commit -m "auto commit from makefile"
	@echo "$(CYAN)Pushing changes...$(RESET)"
	@git push
	@echo "$(GREEN)Git operations completed successfully!$(RESET)"

.PHONY: all clean fclean re debug run test git nc irssi 
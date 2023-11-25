CC = gcc
CFLAGS = -Wall -Wextra -Werror -lreadline
SRC_DIR = .
OBJ_DIR = objs
INCLUDE_DIR = .
BIN_NAME = myshell

SRCS = main.c execute.c bg_seq.c history.c prompt.c signal.c tokenizer.c wildcard.c
OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(SRCS))

.PHONY: all clean fclean re

all: print_ascii_art $(BIN_NAME)

print_ascii_art:
	@echo "    __  ____     _______ _    _ ______ _      _      "
	@echo "   |  \/  \ \   / / ____| |  | |  ____| |    | |     "
	@echo "   | \  / |\ \_/ / (___ | |__| | |__  | |    | |     "
	@echo "   | |\/| | \   / \___ \|  __  |  __| | |    | |     "
	@echo "   | |  | |  | |  ____) | |  | | |____| |____| |____ "
	@echo "   |_|  |_|  |_| |_____/|_|  |_|______|______|______|"
	@echo "                                                     "

$(BIN_NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lreadline

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(BIN_NAME)

re: fclean all


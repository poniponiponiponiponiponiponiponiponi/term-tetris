GAME := tetris
SERVER := server

CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -march=native --std=gnu11 -ggdb -Wstrict-aliasing -fsanitize=address 
LDFLAGS = -lncurses -lmenu -lform -fsanitize=address

BUILD_DIR := ./build
SRC_DIRS := ./src

GAME_SRCS := $(shell find $(SRC_DIRS) ! -name 'server.c' -name '*.c')
GAME_OBJS := $(GAME_SRCS:%=$(BUILD_DIR)/%.o)

SERVER_SRCS := $(shell find $(SRC_DIRS) ! -name 'tetris.c' -name '*.c')
SERVER_OBJS := $(SERVER_SRCS:%=$(BUILD_DIR)/%.o)

all: $(SERVER) $(GAME)

$(SERVER): $(SERVER_OBJS)
	$(CC) $(SERVER_OBJS) -o $@ $(LDFLAGS)

$(GAME): $(GAME_OBJS)
	$(CC) $(GAME_OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm $(GAME)
	rm $(SERVER)
	rm -r $(BUILD_DIR)

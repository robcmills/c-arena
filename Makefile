# Arena Game Engine Makefile

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -fPIC -O2
DEBUG_FLAGS = -g -DDEBUG -O0

SRC_DIR = src/core
RENDER_DIR = src/render
BUILD_DIR = build
LIB_DIR = lib

# Source files
SRCS = $(SRC_DIR)/arena.c \
       $(SRC_DIR)/player.c \
       $(SRC_DIR)/combat.c \
       $(SRC_DIR)/game.c \
       $(SRC_DIR)/api.c

OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Render source files
RENDER_SRCS = $(RENDER_DIR)/render.c $(RENDER_DIR)/screenshot.c
RENDER_OBJS = $(RENDER_SRCS:$(RENDER_DIR)/%.c=$(BUILD_DIR)/%.o)

# Library name
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LIB_EXT = dylib
    LIB_FLAGS = -dynamiclib
    SDL_CFLAGS = $(shell sdl2-config --cflags 2>/dev/null || echo "-I/opt/homebrew/include/SDL2 -I/usr/local/include/SDL2")
    SDL_LDFLAGS = $(shell sdl2-config --libs 2>/dev/null || echo "-L/opt/homebrew/lib -L/usr/local/lib -lSDL2") -lSDL2_image
else
    LIB_EXT = so
    LIB_FLAGS = -shared
    SDL_CFLAGS = $(shell sdl2-config --cflags)
    SDL_LDFLAGS = $(shell sdl2-config --libs) -lSDL2_image
endif

LIB_NAME = libarena.$(LIB_EXT)
RENDER_BIN = $(BUILD_DIR)/arena_render

# Targets
.PHONY: all clean debug test dirs render

all: dirs $(LIB_DIR)/$(LIB_NAME)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: all

dirs:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(LIB_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_DIR)/$(LIB_NAME): $(OBJS)
	$(CC) $(LIB_FLAGS) -o $@ $(OBJS)

# Render target
render: dirs $(RENDER_BIN)

$(BUILD_DIR)/render.o: $(RENDER_DIR)/render.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(BUILD_DIR)/screenshot.o: $(RENDER_DIR)/screenshot.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(BUILD_DIR)/main_render.o: $(RENDER_DIR)/main.c
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -I$(SRC_DIR) -c $< -o $@

$(RENDER_BIN): $(OBJS) $(BUILD_DIR)/render.o $(BUILD_DIR)/screenshot.o $(BUILD_DIR)/main_render.o
	$(CC) $(CFLAGS) -o $@ $^ $(SDL_LDFLAGS)

# Test runner
TEST_DIR = tests
TEST_SRCS = $(wildcard $(TEST_DIR)/*.c)
TEST_BIN = $(BUILD_DIR)/test_runner

test: debug $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_DIR)/test_main.c $(OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -I$(SRC_DIR) -o $@ $^

clean:
	rm -rf $(BUILD_DIR) $(LIB_DIR)

# Show info
info:
	@echo "Platform: $(UNAME_S)"
	@echo "Library: $(LIB_NAME)"
	@echo "Sources: $(SRCS)"
	@echo "Objects: $(OBJS)"
	@echo "SDL CFLAGS: $(SDL_CFLAGS)"
	@echo "SDL LDFLAGS: $(SDL_LDFLAGS)"

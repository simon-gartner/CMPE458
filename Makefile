OS := $(shell uname 2>/dev/null || echo Windows)

CC = gcc
CFLAGS = -Iphase1-w25/include -Wall -Wextra -std=c11
SRC := $(shell find phase1-w25/src -type f -name "*.c")
OBJ := $(patsubst phase1-w25/src/%.c, build/%.o, $(SRC))
EXEC = build/compilerma

ifeq ($(OS), Windows)
    EXEC := build/lexer.exe
    RM := del /F /Q
else
    RM := rm -rf
endif

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: phase1-w25/src/%.c | build
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<


build:
	mkdir -p build

clean:
	$(RM) build/*

.PHONY: all clean build
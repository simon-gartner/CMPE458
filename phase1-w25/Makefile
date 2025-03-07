CC = gcc
CFLAGS = -Iphase1-w25/include -Wall -Wextra

SRC := $(shell find src/ -type f -name "*.c")
OBJ := $(patsubst src/%.c, build/%.o, $(SRC))

EXEC = build/compiler

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c | build
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

build:
	mkdir -p build

clean:
	rm -rf build/*

.PHONY: all clean build

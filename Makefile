CC = gcc
CFLAGS = -Wall -Wextra -Wunused -g -Iinclude
DBVIEW_TARGET = bin/dbview
CLIENT_TARGET = bin/client
TARGETS = $(DBVIEW_TARGET) $(CLIENT_TARGET)

DBVIEW_SRC = src/main.c src/parse.c src/file.c src/server.c
CLIENT_SRC = src/client.c

DBVIEW_OBJ = $(patsubst src/%.c, obj/%.o, $(DBVIEW_SRC))
CLIENT_OBJ = $(patsubst src/%.c, obj/%.o, $(CLIENT_SRC))

.PHONY: all clean run default

all: clean $(DBVIEW_TARGET) $(CLIENT_TARGET) run

default: all

run: $(DBVIEW_TARGET)
	mkdir -p obj bin
	./$(DBVIEW_TARGET) -f ./mynewdb.db -n

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(CLIENT_TARGET): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(DBVIEW_TARGET): $(DBVIEW_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

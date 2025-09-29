CC = gcc
CFLAGS = -Wall -g
DBVIEW_TARGET = bin/dbview
SERVER_TARGET = bin/dbserver
TARGETS = $(DBVIEW_TARGET) $(SERVER_TARGET)

DBVIEW_SRC = src/main.c src/parse.c src/file.c
SERVER_SRC = src/server.c

DBVIEW_OBJ = $(patsubst src/%.c, obj/%.o, $(DBVIEW_SRC))
SERVER_OBJ = $(patsubst src/%.c, obj/%.o, $(SERVER_SRC))

.PHONY: all clean run default

all: clean $(TARGETS) run

default: all

run: $(DBVIEW_TARGET)
	mkdir -p obj bin
	./$(DBVIEW_TARGET) -f ./mynewdb.db -n
	./$(DBVIEW_TARGET) -f ./mynewdb.db -a "Timmy H.,123 Cheshire Ln.,120"

clean:
	rm -f obj/*.o
	rm -f bin/*
	rm -f *.db

$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -Iinclude

$(DBVIEW_TARGET): $(DBVIEW_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -Iinclude

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $< -Iinclude

CC = gcc
CFLAGS = -Wall -Iincludes -Wextra -std=gnu99

all: client server

client: client.c

server: server.c

clean:
	rm -rf client server *.0

.PHONY : clean all

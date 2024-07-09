CC = gcc
CFLAGS = -Wall -Iincludes -Wextra -std=gnu99

all: server

server: server.c

clean:
	rm -rf server *.0

.PHONY : clean all

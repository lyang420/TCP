/* `client.c` is a basic TCP client following IPv4 written in C that sends a
message to `server.c`. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>

#define TRUE 1
#define FALSE 0

int main(int argc, char *argv[]) {
	/* host address configuration */
	const char *port = "8080";
	char *host;
	struct addrinfo hints, *server;
	/* client configuration */
	int r, sock_fd, done;
	char buffer[BUFSIZ];
	fd_set read_fd;

	if (argc < 2) {
		fprintf(stderr, "Usage: client hostname\n");
		exit(1);
	}
	host = argv[1];

	printf("Searching for running servers on %s... ", host);
	memset_s(&hints, sizeof(hints), 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	r = getaddrinfo(host, port, &hints, &server);
	if (r != 0) {
		perror("client getaddrinfo() failed");
		exit(1);
	}
	puts("found");

	/* Create socket. */

	sock_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (sock_fd == -1) {
		perror("client socket() failed");
		exit(1);
	}

	/* Once socket is configured and connected to host address, client can start
	interacting with server. */

	r = connect(sock_fd, server->ai_addr, server->ai_addrlen);
	freeaddrinfo(server);
	if (r == -1) {
		perror("client connect() failed");
		exit(1);
	}

	done = FALSE;
	while (!done) {
		FD_ZERO(&read_fd);
		FD_SET(sock_fd, &read_fd);
		FD_SET(0, &read_fd);
		r = select(sock_fd + 1, &read_fd, NULL, NULL, 0);
		if (r == -1) {
			perror("client select() failed");
			exit(1);
		}
		if (FD_ISSET(sock_fd, &read_fd)) {
			r = recv(sock_fd, buffer, BUFSIZ, 0);
			if (r < 1) {
				puts("client recv() closed");
				break;
			}
			buffer[r] = '\0';
			printf("%s", buffer);
		}
		if (FD_ISSET(0, &read_fd)) {
			if (fgets(buffer, BUFSIZ, stdin) == NULL) {
				putchar('\n');
			} else if (strcmp(buffer, "disconnect\n") == 0) {
				done = TRUE;
			} else {
				send(sock_fd, buffer, strlen(buffer), 0);
			}
		}
	}

	/* Close file descriptors and open sockets before terminating program. */

	close(sock_fd);

	return 0;
}

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
	struct addrinfo hints, *host;
	int r, sockfd, i;
	const int buf_size = 1024;
	char buf[buf_size];

	memset_s(&hints, sizeof(hints), 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	r = getaddrinfo(0, "8080", &hints, &host);
	if (r != 0) {
		perror("client getaddrinfo() failed");
		exit(1);
	}

	/* Create socket. */

	sockfd = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
	if (sockfd == -1) {
		perror("client socket() failed");
		exit(1);
	}

	/* Once socket is configured and connected to host address, client can start
	interacting with server. */

	r = connect(sockfd, host->ai_addr, host->ai_addrlen);
	if (r == -1) {
		perror("client connect() failed");
		exit(1);
	}

	for (;;) {
		printf(" > ");
		fgets(buf, buf_size, stdin);
		if (buf[0] == '\n') {
			break;
		}
		for (i = 0; i < buf_size; i++) {
			if (buf[i] == '\n') {
				buf[i] = '\0';
				break;
			}
		}
		if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0) {
			break;
		}
		r = send(sockfd, buf, strlen(buf), 0);
		if (r == -1) {
			perror("client send() failed");
			exit(1);
		}
		r = recv(sockfd, buf, buf_size, 0);
		buf[r] = '\0';
		printf("%s\n", buf);
	}

	/* Close file descriptors and open sockets before terminating program. */

	freeaddrinfo(host);
	close(sockfd);

	return 0;
}

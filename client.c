/* `client.c` is a basic TCP client following IPv4 written in C that sends a
message to `server.c`. */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
	struct addrinfo hints, *host;
	int r, sockfd;
	const char *msg = "test\n";
	const int buf_size = 1024;
	char buf[buf_size];

	/* Configure localhost address. */

	memset_s(&hints, sizeof(hints), 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	r = getaddrinfo(0, "8080", &hints, &host);
	if (r != 0) {
		perror("Client failed to configure host address");
		exit(1);
	}

	/* Create socket. */

	sockfd = socket(host->ai_family, host->ai_socktype, host->ai_protocol);
	if (sockfd == -1) {
		perror("Failed to create client socket");
		exit(1);
	}

	/* Once socket is configured and connected to host address, client can start
	interacting with server. */

	r = connect(sockfd, host->ai_addr, host->ai_addrlen);
	if (r == -1) {
		perror("Failed to connect client socket");
		exit(1);
	}

	r = send(sockfd, msg, strlen(msg), 0);
	if (r == -1) {
		perror("Failed to send data to server");
		exit(1);
	}

	r = recv(sockfd, buf, buf_size, 0);
	buf[r] = '\0';
	printf("%s\n", buf);

	/* Close file descriptors and open sockets before terminating program. */

	freeaddrinfo(host);
	close(sockfd);
	puts("Finished.");

	return 0;
}

/* `server.c` is a basic TCP server written in C supporting both IPv4 and IPv6
requests. After receiving data from a client, the server sends it back with a
greeting message acknowledging receipt. */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
	struct addrinfo hints, *server;
	struct sockaddr client_address;
	socklen_t client_len;
	int r, sockfd, clientfd, option;
	const int size = 1024;
	char input[size], output[size];

	/* Configure host address, which is the localhost for now. Use `memset_s` and
	not `memset`. */

	memset_s(&hints, sizeof(hints), 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	r = getaddrinfo(0, "8080", &hints, &server);
	if (r != 0) {
		perror("server getaddrinfo() failed");
		exit(1);
	}

	/* Create socket. */

	sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (sockfd == -1) {
		perror("server socket() failed");
		exit(1);
	}

	/* Activate dual stack configuration (notice setting of family to `AF_INET6`
	and not `AF_INET` at line 25 above and usage of `AI_PASSIVE` flag at line
	27). I am told this may not be supported on all operating systems. */

	option = 0;
	r = setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &option, sizeof(option));
	if (r == -1) {
		perror("server setsockopt() failed");
		exit(1);
	}

	/* Once configured, bind server socket to host address and listen for
	incoming requests on the network. */

	r = bind(sockfd, server->ai_addr, server->ai_addrlen);
	if (r == -1) {
		perror("server bind() failed");
		exit(1);
	}

	printf("Server is listening...\n");
	r = listen(sockfd, 1);
	if (r == -1) {
		perror("server listen() failed");
		exit(1);
	}

	/* After accepting request, process into buffer and send it back. */

	client_len = sizeof(client_address);
	clientfd = accept(sockfd, &client_address, &client_len);
	if (clientfd == -1) {
		perror("server accept() failed");
		exit(1);
	}

	for (;;) {
		r = recv(clientfd, input, size, 0);
		if (r > 0) {
			input[r] = '\0';
			strcpy(output, " $ ");
			strcat(output, input);
			r = send(clientfd, output, strlen(output), 0);
			if (r < 1) {
				perror("server send() failed");
				exit(1);
			}
		} else {
			break;
		}
	}

	/* Close file descriptors and open sockets before terminating program. */

	freeaddrinfo(server);
	close(clientfd);
	close(sockfd);

	return 0;
}

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
	const int buf_size = 1024;
	char buf[buf_size];
	const char *greeting = "Hey, you sent me: ";

	/* Configure host address, which is the localhost for now. Use `memset_s` and
	not `memset`. */

	memset_s(&hints, sizeof(hints), 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	r = getaddrinfo(0, "8080", &hints, &server);
	if (r != 0) {
		perror("Server failed to configure host address");
		exit(1);
	}

	/* Create socket. */

	sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (sockfd == -1) {
		perror("Failed to create server socket");
		exit(1);
	}

	/* Activate dual stack configuration (notice setting of family to `AF_INET6`
	and not `AF_INET` at line 25 above and usage of `AI_PASSIVE` flag at line
	27). I am told this may not be supported on all operating systems. */

	option = 0;
	r = setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &option, sizeof(option));
	if (r == -1) {
		perror("Failed to activate dual stack configuration");
		exit(1);
	}

	/* Once configured, bind server socket to host address and listen for
	incoming requests on the network. */

	r = bind(sockfd, server->ai_addr, server->ai_addrlen);
	if (r == -1) {
		perror("Failed to bind server socket");
		exit(1);
	}

	r = listen(sockfd, 1);
	if (r == -1) {
		perror("Failed listening for incoming requests");
		exit(1);
	}

	/* After accepting request, process into buffer and send it back. */

	client_len = sizeof(client_address);
	clientfd = accept(sockfd, &client_address, &client_len);
	if (clientfd == -1) {
		perror("Failed to accept client socket");
		exit(1);
	}

	r = recv(clientfd, buf, buf_size, 0);
	if (r > 0) {
		buf[r] = '\0';
		r = send(clientfd, greeting, strlen(greeting), 0);
		r = send(clientfd, buf, strlen(buf), 0);
	}

	/* Close file descriptors and open sockets before terminating program. */

	freeaddrinfo(server);
	close(clientfd);
	close(sockfd);

	return 0;
}

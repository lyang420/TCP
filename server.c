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

	printf("Configuring server... ");
	memset_s(&hints, sizeof(hints), 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	r = getaddrinfo(0, "8080", &hints, &server);
	if (r != 0) {
		perror("Failed to configure host address");
		exit(1);
	}
	puts("done.");

	printf("Configuring socket... ");
	sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (sockfd == -1) {
		perror("Failed to create server socket");
		exit(1);
	}
	puts("done.");

	option = 0;
	r = setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, (void *) &option, sizeof(option));
	if (r == -1) {
		perror("Failed to activate dual stack configuration");
		exit(1);
	}

	printf("Binding socket... ");
	r = bind(sockfd, server->ai_addr, server->ai_addrlen);
	if (r == -1) {
		perror("Failed to bind server socket");
		exit(1);
	}
	puts("done.");

	printf("Listening for incoming requests...\n");
	r = listen(sockfd, 1);
	if (r == -1) {
		perror("Failed listening for incoming requests");
		exit(1);
	}

	printf("Accepting new connection ");
	client_len = sizeof(client_address);
	clientfd = accept(sockfd, &client_address, &client_len);
	if (clientfd == -1) {
		perror("Failed to accept client socket");
		exit(1);
	}
	printf("on %d\n", clientfd);

	r = recv(clientfd, buf, buf_size, 0);
	if (r > 0) {
		buf[r] = '\0';
		printf("Received %d bytes:\n---\n", r);
		for (int i = 0; i < r; i++) {
			putchar(buf[i]);
		}
		r = send(clientfd, buf, strlen(buf), 0);
		if (r < 1) {
			perror("Failed to echo client");
			exit(1);
		}
		printf("Sent %d bytes\n", r);
	}

	freeaddrinfo(server);
	close(clientfd);
	close(sockfd);
	puts("Finished.");

	return 0;
}

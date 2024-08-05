/* `server.c` is a basic TCP server written in C supporting both IPv4 and IPv6
requests. After receiving data from a client, the server sends it back with a
greeting message acknowledging receipt. */

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>

#define TRUE 1
#define FALSE 0

int main() {
	const char *port = "8080";
	const int clientname_size = 32;
	char clientname[clientname_size];
	char buffer[BUFSIZ], sendstr[BUFSIZ];
	const int backlog = 10;
	char connection[backlog][clientname_size];
	socklen_t address_len = sizeof(struct sockaddr);
	struct addrinfo hints, *server;
	struct sockaddr address;
	int r, max_connect, fd, i, done;
	fd_set main_fd, read_fd;
	int serverfd, clientfd;

	/* Configure host address, which is the localhost for now. Use `memset_s` and
	not `memset`. */

	memset_s(&hints, sizeof(hints), 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	r = getaddrinfo(0, port, &hints, &server);
	if (r != 0) {
		perror("server getaddrinfo() failed");
		exit(1);
	}

	/* Create socket. */

	serverfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (serverfd == -1) {
		perror("server socket() failed");
		exit(1);
	}

	/* Once configured, bind server socket to host address and listen for
	incoming requests on the network. */

	r = bind(serverfd, server->ai_addr, server->ai_addrlen);
	if (r == -1) {
		perror("server bind() failed");
		exit(1);
	}

	printf("Server is listening...\n");
	r = listen(serverfd, backlog);
	if (r == -1) {
		perror("server listen() failed");
		exit(1);
	}

	max_connect = backlog;
	FD_ZERO(&main_fd);
	FD_SET(serverfd, &main_fd);

	done = FALSE;
	while (!done) {
		read_fd = main_fd;
		r = select(max_connect + 1, &read_fd, NULL, NULL, 0);
		if (r == -1) {
			perror("server select() failed");
			exit(1);
		}
		for (i = 1; i <= max_connect; i++) {
			if (FD_ISSET(fd, &read_fd)) {
				if (fd == serverfd) {
					clientfd = accept(serverfd, (struct sockaddr *) &address, &address_len);
					if (clientfd == -1) {
						perror("server accept() failed");
						exit(1);
					}
					r = getnameinfo((struct sockaddr *) &address, address_len,
										 clientname, clientname_size, 0, 0, NI_NUMERICHOST);
					strcpy(connection[clientfd], clientname);
					FD_SET(clientfd, &main_fd);
					strcpy(buffer, " > ");
					strcat(buffer, connection[clientfd]);
					strcat(buffer, " connected to the server\n");
					strcat(buffer, " > Type 'disconnect' to disconnect; 'exit' to stop\n");
					send(clientfd, buffer, strlen(buffer), 0);
					for (i = serverfd + 1; i < max_connect; i++) {
						if (FD_ISSET(i, &main_fd)) {
							send(i, buffer, strlen(buffer), 0);
						}
					}
					printf("%s", buffer);
				} else {
					r = recv(fd, buffer, BUFSIZ, 0);
					if (r < 1) {
						FD_CLR(fd, &main_fd);
						close(fd);
						strcpy(buffer, " > ");
						strcat(buffer, connection[fd]);
						strcat(buffer, " disconnected\n");
						for (i = serverfd + 1; i < max_connect; i++) {
							if (FD_ISSET(i, &main_fd)) {
								send(i, buffer, strlen(buffer), 0);
							}
						}
						printf("%s", buffer);
					} else {
						buffer[r] = '\0';
						if (strcmp(buffer, "'exit\n") == 0) {
							done = TRUE;
						} else {
							strcpy(sendstr, connection[fd]);
							strcat(sendstr, " > ");
							strcat(sendstr, buffer);
							for (i = serverfd + 1; i < max_connect; i++) {
								if (FD_ISSET(i, &main_fd)) {
									send(i, sendstr, strlen(sendstr), 0);
								}
							}
							printf("%s", sendstr);
						}
					}
				}
			}
		}
	}

	puts(" > Shutting down server");
	close(serverfd);
	freeaddrinfo(server);

	return 0;
}

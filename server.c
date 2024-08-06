/* `server.c` is a TCP server written in C supporting IPv4 requests from
multiple clients (though not concurrently). After receiving data from a client,
the server announces to other connections on the local network that a new
participant has joined, and sends back messages acknowledging receipt. The
server may terminate connection from a client provided they enter the
appropriate input. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/select.h>

#define TRUE 1
#define FALSE 0

int main() {
	/* host address configuration */
	const char *port = "8080";
	struct addrinfo hints, *server;
	/* server and client socket configuration */
	int server_fd, client_fd;
	/* client info configuration */
	const int clientname_size = 32;
	char clientname[clientname_size];
	char buffer[BUFSIZ], sendstr[BUFSIZ];
	/* support for multiple clients */
	const int backlog = 10;
	char connection[backlog][clientname_size];
	socklen_t address_len = sizeof(struct sockaddr);
	struct sockaddr address;
	int r, max_num_clients, request_fd, done, i;
	fd_set main_fd, read_fd;

	/* Configure host address, which is the localhost for now. Use `memset_s` and
	not `memset`. */

	memset_s(&hints, sizeof(hints), 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	r = getaddrinfo(0, port, &hints, &server);
	if (r != 0) {
		perror("server getaddrinfo() failed");
		exit(1);
	}

	/* Create socket. */

	server_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (server_fd == -1) {
		perror("server socket() failed");
		exit(1);
	}

	/* Once configured, bind server socket to host address and listen for
	incoming requests on the network. */

	r = bind(server_fd, server->ai_addr, server->ai_addrlen);
	if (r == -1) {
		perror("server bind() failed");
		exit(1);
	}

	printf("Server is listening...\n");
	r = listen(server_fd, backlog);
	if (r == -1) {
		perror("server listen() failed");
		exit(1);
	}

	max_num_clients = backlog;
	FD_ZERO(&main_fd);
	FD_SET(server_fd, &main_fd);

	done = FALSE;
	while (!done) {
		read_fd = main_fd;
		r = select(max_num_clients + 1, &read_fd, NULL, NULL, 0);
		if (r == -1) {
			perror("server select() failed");
			exit(1);
		}
		for (i = 1; i <= max_num_clients; i++) {
			if (FD_ISSET(request_fd, &read_fd)) {
				if (request_fd == server_fd) {
					client_fd = accept(server_fd, (struct sockaddr *) &address, &address_len);
					if (client_fd == -1) {
						perror("server accept() failed");
						exit(1);
					}
					r = getnameinfo((struct sockaddr *) &address, address_len,
										 clientname, clientname_size, 0, 0, NI_NUMERICHOST);
					strcpy(connection[client_fd], clientname);
					FD_SET(client_fd, &main_fd);
					strcpy(buffer, " > ");
					strcat(buffer, connection[client_fd]);
					strcat(buffer, " connected to the server\n");
					strcat(buffer, " > Type 'disconnect' to disconnect; 'exit' to stop\n");
					send(client_fd, buffer, strlen(buffer), 0);
					for (i = server_fd + 1; i < max_num_clients; i++) {
						if (FD_ISSET(i, &main_fd)) {
							send(i, buffer, strlen(buffer), 0);
						}
					}
					printf("%s", buffer);
				} else {
					r = recv(request_fd, buffer, BUFSIZ, 0);
					if (r < 1) {
						FD_CLR(request_fd, &main_fd);
						close(request_fd);
						strcpy(buffer, " > ");
						strcat(buffer, connection[request_fd]);
						strcat(buffer, " disconnected\n");
						for (i = server_fd + 1; i < max_num_clients; i++) {
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
							strcpy(sendstr, connection[request_fd]);
							strcat(sendstr, " > ");
							strcat(sendstr, buffer);
							for (i = server_fd + 1; i < max_num_clients; i++) {
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
	close(server_fd);
	freeaddrinfo(server);

	return 0;
}

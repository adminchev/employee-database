#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>

#define PORT 5555
#define BACKLOG 10
#define MAX_CLIENTS 10

int main(int argc, char *argv[]) {
	
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in server_info;
	memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = htonl(INADDR_ANY);
	server_info.sin_port = htons(PORT);
	struct pollfd fds[MAX_CLIENTS + 1];
	int nfds = 0;

	if (bind(listen_fd, (struct sockaddr *)&server_info, sizeof(server_info)) == -1) {
		perror("bind");
		return -1;
	};
	
	   if (listen(listen_fd, BACKLOG) == -1) {
		perror("listen");
		return -1;
	};

	fds[0].fd = listen_fd;
	fds[0].events = POLLIN;
	nfds = 1;

	while(1) {
		int poll_count = poll(fds, nfds, -1);
		if (poll_count < 0) {
			perror("poll");
			return -1;
		} else if (poll_count == 0) {
			continue;
		} else {
			for (int i = 0; i < nfds; i++) {

				if (fds[i].revents & POLLIN) {

					if (fds[i].fd == listen_fd) {
						int new_conn = accept(listen_fd, NULL, NULL);

						if (new_conn == -1) {
							perror("accept");
							continue;

						}   else {
							
							if (nfds < MAX_CLIENTS + 1) {
								fds[nfds].fd = new_conn;
								fds[nfds].events = POLLIN;
								nfds++;
							} else {
								close(new_conn);
								printf("Too many connections. Exceeded %d\n", MAX_CLIENTS);
							};
						};
					
					} else {
						char buffer[256];
						int nbytes = recv(fds[i].fd, buffer, sizeof(buffer), 0);

						if (nbytes <= 0) {
							if (nbytes == 0) {
								printf("Client on fd %d disconnected.\n", fds[i].fd);
							} else {
								perror("recv");
							};
							close(fds[i].fd);
							fds[i] = fds[nfds -1];
							nfds--;
							i--;
					
						} else {
							printf("Data on fd %d: %s\n", fds[i].fd, buffer);
						};
					};
				};
			};
		};
	};

    return 0;
}

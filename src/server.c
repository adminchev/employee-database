#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5555
#define BACKLOG 10

int main(int argc, char *argv[]) {
	
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = 1;
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1){
		perror("setsockopt");
	};
	
	struct sockaddr_in server_info;
	memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = htonl(INADDR_ANY);
	server_info.sin_port = htons(PORT);
	socklen_t addr_len = sizeof(server_info);

	if (bind(listen_fd, (struct sockaddr *)&server_info, sizeof(server_info)) == -1) {
		perror("bind");
		return -1;
	};
	
	   if (listen(listen_fd, BACKLOG) == -1) {
		perror("listen");
		return -1;
	};

	fd_set master_set;
	fd_set read_fds;

	FD_ZERO(&master_set);
	FD_ZERO(&read_fds);
	FD_SET(listen_fd, &master_set);
	int max_fd = listen_fd;

	while(1) {
		read_fds = master_set;
		if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(1);
		};
		int client_fd = -1;
		for (int i = 0; i <= max_fd; i++) {
			if (FD_ISSET(i, &read_fds)) {
				if (i == listen_fd){
					client_fd = accept(i, (struct sockaddr *)&server_info, &addr_len);
					if (client_fd == -1) {
						perror("accept");
					} else {
						FD_SET(client_fd, &master_set);
						if (client_fd > max_fd) {
							max_fd = client_fd;
						};
					};
				} else {
					char buffer[1024];
					int sock_data = read(i, buffer, sizeof(buffer));
					if (sock_data <= 0) {
						if (sock_data == 0) {
							printf("Socket %d hung up\n", i);
						} else {
							perror("read");
						};
						close(i);
						FD_CLR(i, &master_set);
					} else {
						printf("sock %d: %.*s\n", i, sock_data, buffer);
					};
				}	
			};
		};
	};
    return 0;
}

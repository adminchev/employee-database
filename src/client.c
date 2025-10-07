#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		printf("Usage: %s <IP address>\n", argv[0]);
		return 0;
	};

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in server_info;
	memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = inet_addr(argv[1]);
	server_info.sin_port = htons(PORT);

	if (connect(fd, (struct sockaddr *)&server_info, sizeof(server_info)) == -1) {
	    perror("connect");
		close(fd);
	};

    return 0;

}

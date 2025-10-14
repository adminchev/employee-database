#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"

static const cmd_mapping_t cmd_map[] = {
	{"list", CMD_LIST_EMPLOYEES},
	{NULL, 0}
};

int handle_read(int fd) {
    char buffer[BUFF_SIZE];
	if (read(fd, buffer, BUFF_SIZE) == -1) {
		perror("read");
		return -1;
	};
	printf("%s\n", buffer);
	return 0;
};

int handle_interractive_write(int fd, request_t request, cmd_type_e cmd) {
	request.cmd = cmd;

	if (write(fd, &request, sizeof(request_t)) == -1) {
		perror("write");
	};
	handle_read(fd);
	return 0;
};

int handle_write(int fd, request_t request) {
	request.cmd = CMD_LIST_EMPLOYEES;
	request.len = 0;

	if (write(fd, &request, sizeof(request_t)) == -1) {
		perror("write");
	};

	handle_read(fd);
	return 0;
};

cmd_type_e cmd_parser(char *user_input) {
	for (int i = 0; cmd_map[i].user_cmd != NULL; i++){
		if (strcmp(user_input, cmd_map[i].user_cmd) == 0) {
			return cmd_map[i].server_cmd;
		};
	};
	return -1;
};


int conn_client(char *ip_str, request_t request) {
	if (ip_str == NULL) {
		printf("Usage: %s <IP address>\n", ip_str);
		return -1;
	};

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in server_info;
	memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = inet_addr(ip_str);
	server_info.sin_port = htons(PORT);

	if (connect(fd, (struct sockaddr *)&server_info, sizeof(server_info)) == -1) {
	    perror("connect");
		close(fd);
		return -1;
	};

	handle_write(fd, request);

    return fd;
};

int interractive_mode(int fd) {
	char input[256];
	request_t request = {0};

	while(1) {
		printf("> ");
		if (fgets(input, sizeof(input), stdin) == NULL) {
			break;
		};
		
		size_t len = strlen(input);
		if (len > 0 && input[len - 1] == '\n') {
			input[len - 1] = '\0';
		};
		cmd_type_e cmd = cmd_parser(input);
		handle_interractive_write(fd, request, cmd);


	};
};

int main(int argc, char *argv[]) {
	request_t request = {0};
	int c = 0;
	char *ip_str = NULL;
	int interractive = 0;
	int fd = -1;
	
	while ( (c = getopt(argc, argv, "c:il")) != -1) {
		switch (c) {
			case 'c':
				ip_str = optarg;
				break;
			
			case 'i':
				interractive = 1;
				break;

			case 'l':
				request.cmd = CMD_LIST_EMPLOYEES;
				break;
		};
	};

	fd = conn_client(ip_str, request);
	if (fd == -1) {
		printf("Could not connect to server.\n");
		return -1;
	};
	
	if (interractive == 1) {
		interractive_mode(fd);
	};

	return 0;
}

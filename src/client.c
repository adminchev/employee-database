#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "common.h"

static const cmd_mapping_t cmd_map[] = {
	{"list", CMD_LIST_EMPLOYEES},
	{"add", CMD_ADD_EMPLOYEE},
	{"del", CMD_DELETE_EMPLOYEE},
	{NULL, 0}
};

int handle_read(int fd) {
    char buffer[BUFF_SIZE] = {0};
	if (read(fd, buffer, BUFF_SIZE) == -1) {
		perror("read");
		return -1;
	};
	printf("%s\n", buffer);
	return 0;
};

int handle_interactive_write(int fd, request_t request) {
	request.cmd = htonl(request.cmd);
	request.len = htonl(request.len);

	if (write(fd, &request, sizeof(request_t)) == -1) {
		perror("write");
	};
	handle_read(fd);
	return 0;
};

int handle_write(int fd, request_t request) {
	request.cmd = htonl(request.cmd);
    request.len = htonl(request.len);
	if (write(fd, &request, sizeof(request_t)) == -1) {
		perror("write");
	};

	handle_read(fd);
	return 0;
};

int cmd_parser(char *user_input, request_t *request) {
	char *user_input_cmd = strtok(user_input, " ");
	char *user_input_data = strtok(NULL, "");
	if (user_input_cmd == NULL) {
		printf("Could not get command.\n");
		return -1;
	};

	int found = 0;
	for (int i = 0; cmd_map[i].user_cmd != NULL; i++){
		if (strcmp(user_input_cmd, cmd_map[i].user_cmd) == 0) {
			request->cmd = cmd_map[i].server_cmd;
			found = 1;
			break;
		};
	};
	
	if (found == 0) {
		return -1;
	};

	if (user_input_data != NULL) {
		int len = strlen(user_input_data);
		if (len > 0) {
			request->len = len;
			strncpy(request->data, user_input_data, BUFF_SIZE - 1);
			request->data[BUFF_SIZE - 1] = '\0';
		};
	};
	return 0;
};


int conn_client(char *ip_str, request_t request) {
	(void)request;
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

    return fd;
};

int interactive_mode(int fd) {
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

		int result = cmd_parser(input, &request);
		if (result == -1) {
			printf("Invalid command.\n");
			continue;
		};
		handle_interactive_write(fd, request);


	};
	return 0;
};

int main(int argc, char *argv[]) {
	request_t request = {0};
	int c = 0;
	char *ip_str = NULL;
	int interactive = 0;
	int fd = -1;
    int cli_args = 0;

	while ( (c = getopt(argc, argv, "c:il")) != -1) {
		switch (c) {
			case 'c':
				ip_str = optarg;
				break;
			
			case 'i':
				interactive = 1;
				break;

			case 'l':
				request.cmd = CMD_LIST_EMPLOYEES;
				cli_args = 1;
				break;
		};
	};

	fd = conn_client(ip_str, request);
	if (fd == -1) {
		printf("Could not connect to server.\n");
		return -1;
	};

	if (cli_args == 1) {
		handle_write(fd, request);
	};
	
	if (interactive == 1) {
		interactive_mode(fd);
	};
	close(fd);
	return 0;
}

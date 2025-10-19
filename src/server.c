#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <arpa/inet.h>
#include <stdint.h>

#include "common.h"
#include "file.h"
#include "parse.h" 

#define BACKLOG 10

void initialize_clients(clientstate_t *state) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		state[i].fd = -1;
		state[i].state = STATE_NEW;
		state[i].bytes_received = 0;
		memset(&state[i].request, 0, sizeof(request_t));
	};
}

int find_free_slot(clientstate_t *state) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (state[i].fd == -1) {
			return i;
		}
	}
	printf("Max connections reached.\n");
	return -1;
}

int find_slot_by_fd(clientstate_t *state, int fd) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (state[i].fd == fd) {
			return i;
		}
	}
	return -1;
}

int free_slot(clientstate_t *state, int fd) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (state[i].fd == fd) {
			state[i].fd = -1;
			state[i].state = STATE_DISCONNECTED;
			state[i].bytes_received = 0;
			memset(&state[i].request, '\0', sizeof(request_t));
			return 0;
		}
	}
	return -1; 
}

int handle_list_cmd(struct dbheader_t *dbhdr, clientstate_t *state, struct employee_t *employees) {
	if (dbhdr == NULL) {
		printf("Invalid refrence to header or to employees struct\n");
		return -1;
	};

	char response[BUFF_SIZE] = {0};
	size_t offset = 0;

	offset += snprintf(response + offset, BUFF_SIZE - offset, "Number of employees: %d\n", dbhdr->count);

	for (int i = 0; i < dbhdr->count; i++) {
		offset += snprintf(response + offset, BUFF_SIZE - offset, "\tname: %s, address: %s, hours: %d\n", employees[i].name, employees[i].address, employees[i].hours);
		if (offset >= BUFF_SIZE) {
			offset = BUFF_SIZE - 1;
			printf("Buffer memory exhaused.\n");
			break;
		};
	};

	if (strlen(response) <= offset){
		response[BUFF_SIZE -1] = '\0';
	};

	if (send(state->fd, response, offset, 0) == -1){
		perror("send");
		return -1;
	};
	return offset;
	
};

int handle_add_employee(struct dbheader_t *dbhdr, clientstate_t *state, struct employee_t **employees) {
	if (dbhdr == NULL || state == NULL) {
		printf("Null pointer passed, exiting.\n");
		return -1;
	};
	char response[BUFF_SIZE] = {0};
	if (add_employee(dbhdr, employees, state->request.data, response) == -1){
		printf("Failed to add employee.\n");
		return -1;
	}; 

	if (send(state->fd, response, strlen(response), 0) == -1){
		perror("send");
		return -1;
	};
	return 0;

};

int handle_delete_employee(struct dbheader_t *dbhdr, clientstate_t *state, struct employee_t **employees) {
	if (dbhdr == NULL || state == NULL) {
		printf("Null pointer passed, exiting.\n");
		return -1;
	};

	if (employees == NULL) {
		printf("No employees, exiting.\n");
		return -1;
	};
    
	char response[BUFF_SIZE] = {0};
	if (delete_employee(dbhdr, employees, state->request.data, response) == 0) {
		dbhdr->count--;
	} else {
		printf("Could not delete employee.\n");
		return -1;
	};

	if (send(state->fd, response, strlen(response), 0) == -1){
		perror("send");
		return -1;
	};
	return 0;
};

int handle_read(int dbfd, int *nbytes, struct dbheader_t *dbhdr, struct employee_t **employees, clientstate_t *state) {
 	*nbytes = recv(state->fd, &state->request, sizeof(request_t), 0);
	if (*nbytes > 0) {

		state->request.cmd = ntohl(state->request.cmd);
		state->request.len = ntohl(state->request.len);
		state->bytes_received = *nbytes;

		switch (state->request.cmd) {
			case CMD_LIST_EMPLOYEES:
				handle_list_cmd(dbhdr, state, *employees);						
				break;

			case CMD_ADD_EMPLOYEE:
				if (handle_add_employee(dbhdr, state, employees) == 0){
					output_file(dbfd, dbhdr, *employees);
				};
				break;

			case CMD_DELETE_EMPLOYEE:
				if(handle_delete_employee(dbhdr, state, employees) == 0) {
					output_file(dbfd, dbhdr, *employees);
				};
				break;

			default:
				printf("Invalid command.\n");
	
		};
	};

	return *nbytes;
};

int server_loop(int dbfd, struct dbheader_t *dbhdr, struct employee_t *employees) {
    
	clientstate_t state[MAX_CLIENTS] = {0};
	int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == -1) {
		perror("socket");
		return -1;
	};
	
	struct sockaddr_in server_info;
	memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
	server_info.sin_addr.s_addr = htonl(INADDR_ANY);
	server_info.sin_port = htons(PORT);
	struct pollfd fds[MAX_CLIENTS + 1] = {0};
	int nfds = 0;
	int opt = 1;
	int slot = -1;

	initialize_clients(state);

	// Allow port reuse - prevents "Address already in use" errors
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		perror("setsockopt");
		return -1;
	}

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

						} else {
							slot = find_free_slot(state);
							if (slot == -1 || nfds >= MAX_CLIENTS + 1) {
								close(new_conn);
								printf("Too many connections. Exceeded %d\n", MAX_CLIENTS);
								continue;
							};
							state[slot].fd = new_conn;
							state[slot].state = STATE_NEW;
							memset(state[slot].request.data, '\0', BUFF_SIZE);
							fds[nfds].fd = new_conn;
							fds[nfds].events = POLLIN;
							nfds++;
						};
					
					} else {
						int nbytes = 0;
						if ((slot = find_slot_by_fd(state, fds[i].fd)) != -1) {
							memset(state[slot].request.data, '\0', BUFF_SIZE);
							if (handle_read(dbfd, &nbytes, dbhdr, &employees, &state[slot]) == 0) {
								state[slot].state = STATE_CONNECTED;
							};

							if (nbytes <= 0) {
								if (nbytes == 0) {
									printf("Client on fd %d disconnected.\n", fds[i].fd);
								} else {
									perror("recv");
								};
								free_slot(state, fds[i].fd);
								close(fds[i].fd);
								fds[i] = fds[nfds -1];
								memset(&fds[nfds - 1], 0, sizeof(fds[nfds - 1])); // Prevent corruption from reusing old pollfd struct
								fds[i].revents = 0;
								nfds--;
								i--;
							};

						} else {
							printf("Max connections reached.\n");
							continue;
						};

					};
				};
			};
		};
	};

    return 0;
}

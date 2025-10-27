#include <stdint.h>

#ifndef COMMON_H
#define COMMON_H

#define VERSION 1
#define STATUS_ERROR -1
#define STATUS_SUCCESS 0
#define PORT 5555
#define MAX_CLIENTS 10
#define BUFF_SIZE 512

#define SAFE_FREE(ptr) do { \
	free(ptr); \
	ptr = NULL; \
} while(0)

typedef enum {
	MSG_HELLO,
	CMD_CREATE_DB,
	CMD_LIST_EMPLOYEES,
	CMD_ADD_EMPLOYEE,
	CMD_DELETE_EMPLOYEE,
} cmd_type_e;

typedef struct {
	const char *user_cmd;
	cmd_type_e server_cmd;
} cmd_mapping_t;

typedef struct {
	cmd_type_e cmd;
	uint32_t len;
	char data[BUFF_SIZE];
} request_t;

typedef struct {
	cmd_type_e type;
	uint32_t len;
} proto_hdr_t;

typedef enum {
	S_NEW,
	S_CONNECTED,
	S_DISCONNECTED,
	S_HELLO,
	S_MSG,
	S_WAIT_FOR_PACKET,
	S_GOODBYE,
} state_e;

typedef struct {
	int fd;
	state_e state;
	size_t bytes_received;
	char buffer[sizeof(request_t)];
	time_t last_activity;
} clientstate_t;

#endif

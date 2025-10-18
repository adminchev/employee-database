#ifndef SERVER_H
#define SERVER_H
#include "parse.h"

int server_loop(int fd, struct dbheader_t *dbhdr, struct employee_t *employees);

#endif // !SERVER_H

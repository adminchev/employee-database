#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "file.h"
#include "common.h"


int create_db_file(char *filename) {
  int dbfd;
  dbfd = open(filename, O_RDONLY);
  if (dbfd != -1) {
    perror("open");
    printf("File already exists\n");
    return STATUS_ERROR;
  };

  dbfd = open(filename, O_RDWR | O_CREAT, 0640);
  if (dbfd == -1) {
    perror("open");
    return STATUS_ERROR;
  };
  return dbfd;
}

int open_db_file(char *filename) {
  int dbfd;
  dbfd = open(filename, O_RDONLY);
  if (dbfd == -1) {
    perror("open");
    return STATUS_ERROR;
  };
  return dbfd;
}



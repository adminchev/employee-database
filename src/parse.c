#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {

}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {

}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {

}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
  if (fd < 0) {
    printf("Got bad file descriptor\n");
    return STATUS_ERROR;
  };

  dbhdr->magic = ntohl(dbhdr->magic);
  dbhdr->version = ntohs(dbhdr->version);
  dbhdr->count = ntohs(dbhdr->count);
  dbhdr->filesize = ntohl(dbhdr->filesize);
  
  lseek(fd, 0, SEEK_SET);
  write(fd, dbhdr, sizeof(struct dbheader_t));
}	

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  
  if (headerOut == NULL) {
    return STATUS_ERROR;
  };

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if(header == NULL) {
    printf("Could not allocate memory\n");
    return STATUS_ERROR;
  };

  if (read(fd, header, sizeof(struct dbheader_t)) == -1){
    perror("read");
    return STATUS_ERROR;
  };

  header->magic = ntohl(header->magic);
  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->filesize = ntohl(header->filesize);

  if(header->magic != HEADER_MAGIC){
    printf("Invalid database file\n");
    free(header);
    return STATUS_ERROR;
  };

  if(header->version > VERSION) {
    printf("DB file was created on a newer version than the installed\n");
    free(header);
    return STATUS_ERROR;
  };

  struct stat dbfilestat = {0};
  if(fstat(fd, &dbfilestat) == -1) {
    perror("fstat");
    free(header);
    return STATUS_ERROR;
  };

  if (dbfilestat.st_size != header->filesize) {
    printf("Database file size mismatch, possible corruption\n");
    free(header);
    return STATUS_ERROR;
  }

  *headerOut = header;
  return STATUS_SUCCESS;

}

int create_db_header(int fd, struct dbheader_t **headerOut) {
  
  if (headerOut == NULL) {
    printf("headerOut is null");
    return STATUS_ERROR;
  };

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));

  if (header == NULL) {
    printf("Creating new header faied. Unable to allocate memory\n");
    return STATUS_ERROR;
  };

  header->version = 0x1;
  header->magic = HEADER_MAGIC;
  header->count = 0;
  header->filesize = sizeof(struct dbheader_t);
  

  *headerOut = header;

  return STATUS_SUCCESS;

}



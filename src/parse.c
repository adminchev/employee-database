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
  int i = 0;
  for (; i < dbhdr->count; i++) {
    printf("User: %d\n", i);
    printf("\tName: %s\n", employees[i].name);
    printf("\tAddress: %s\n", employees[i].address);
    printf("\tHours: %d\n", employees[i].hours);
  };
};

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring) {

  if (employees == NULL) {
    printf("Failed to create new employee\n");
    return STATUS_ERROR;
  };

  char *name = strtok(addstring, ",");
  char *address = strtok(NULL, ",");
  char *hours_string = strtok(NULL, ",");
  int hours = atoi(hours_string);
  int count = dbhdr->count;
  strncpy((*employees)[count - 1].name, name, sizeof((*employees)[count - 1].name));
  strncpy((*employees)[count - 1].address, address, sizeof((*employees)[count - 1].address));
  (*employees)[count - 1].hours = hours;

  dbhdr->filesize = dbhdr->filesize + sizeof((*employees)[count - 1]);

  return STATUS_SUCCESS;
};

int delete_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *delstring) {
  if (employees == NULL) {
    printf("Failed to delete employee");
    return STATUS_ERROR;
  };

  int employee_index = -1;
  int dbcount = dbhdr->count;
  int i = 0;

  for (; i < dbcount; i++) {
    if (strcmp((*employees)[i].name, delstring) == 0) {
      employee_index = i;
      printf("Deleting employee - %s\n", delstring);
      break;
    };
  };
  
  if (employee_index == -1) {
    printf("Employee not found\n");
    return STATUS_ERROR;
  };

  for (i = employee_index; i < dbcount - 1; i++) {
    (*employees)[i] = (*employees)[i + 1];
  };

  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {
  if (fd < 0) {
    printf("Bad file descriptor passed\n");
    return STATUS_ERROR;
  };

  int count = dbhdr->count;
  struct employee_t *employees = calloc(count, sizeof(struct employee_t));
  if (employees == NULL) {
    printf("Malloc failed\n");
    return STATUS_ERROR;
  };

  if (read(fd, employees, count * sizeof(struct employee_t)) == -1) {
    perror("read");
    return STATUS_ERROR;
  };

  for (int i = 0; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  };

  *employeesOut = employees;
  return STATUS_SUCCESS;

}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
  if (fd < 0) {
    printf("Got bad file descriptor\n");
    return STATUS_ERROR;
  };
  
  int realcount = dbhdr->count;
  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->version = htons(dbhdr->version);
  dbhdr->count = htons(dbhdr->count);
  dbhdr->filesize = htonl(sizeof(struct dbheader_t) + sizeof(struct employee_t) * realcount);
  
  lseek(fd, 0, SEEK_SET);
  write(fd, dbhdr, sizeof(struct dbheader_t));
  
  int i = 0;
  for (; i < realcount; i++){
    employees[i].hours = htonl(employees[i].hours);
    write(fd, &employees[i], sizeof(struct employee_t));
  };
  ftruncate(fd, sizeof(struct dbheader_t) + (realcount * sizeof(struct employee_t)));
  return STATUS_SUCCESS;
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
    printf("st_size - %d, filesize - %d\n", dbfilestat.st_size, header->filesize);
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



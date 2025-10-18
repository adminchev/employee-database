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

int list_employees(struct dbheader_t *dbhdr, struct employee_t *employees, char *response) {
	if (dbhdr == NULL) {
		snprintf(response, BUFF_SIZE, "Listing failed, passed null pointer.\n");
		return -1;
	};

	int i = 0;
	for (; i < dbhdr->count; i++) {
	  printf("User: %d\n", i);
	  printf("\tName: %s\n", employees[i].name);
	  printf("\tAddress: %s\n", employees[i].address);
	  printf("\tHours: %d\n", employees[i].hours);
	};
	return 0;
};

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *addstring, char *response) {

	if (employees == NULL || dbhdr == NULL) {
	  snprintf(response, BUFF_SIZE, "Failed to create new employee\n");
	  return STATUS_ERROR;
	};

	char *name = strtok(addstring, ",");
	char *address = strtok(NULL, ",");
	char *hours_string = strtok(NULL, ",");
	if (name == NULL || address == NULL || hours_string == NULL) {
		snprintf(response, BUFF_SIZE, "Could nor process input, possibly missing field.\n");
		return -1;
	};
	int hours = atoi(hours_string);
	dbhdr->count++;
	int count = dbhdr->count;
	struct employee_t *temp = realloc(*employees, sizeof(struct employee_t) * count);
	if (temp == NULL) {
		snprintf(response, BUFF_SIZE, "Realloc failed.\n");
		return -1;
	}
	*employees = temp;

	strncpy((*employees)[count - 1].name, name, sizeof((*employees)[count - 1].name) - 1);
	(*employees)[count - 1].name[sizeof((*employees)[count - 1].name) - 1] = '\0';
	strncpy((*employees)[count - 1].address, address, sizeof((*employees)[count - 1].address) -1);
	(*employees)[count - 1].address[sizeof((*employees)[count - 1].address) - 1] = '\0';
	(*employees)[count - 1].hours = hours;

	dbhdr->filesize = dbhdr->filesize + sizeof((*employees)[count - 1]);

	snprintf(response, BUFF_SIZE, "Added %s, address: %s\n", name, address);
	
	return STATUS_SUCCESS;
};

int delete_employee(struct dbheader_t *dbhdr, struct employee_t **employees, char *delstring, char *response) {
	if (employees == NULL) {
		snprintf(response, BUFF_SIZE, "Failed to delete employee.\n");
		return STATUS_ERROR;
	};

	int employee_index = -1;
	int dbcount = dbhdr->count;

	for (int i = 0; i < dbcount; i++) {
	  if (strcmp((*employees)[i].name, delstring) == 0) {
	    employee_index = i;
		snprintf(response, BUFF_SIZE, "Deleting employee - %s\n", delstring);	
	    break;
	  };
	};
	
	if (employee_index == -1) {
	  printf("Employee not found\n");
	  return -1;
	};

	for (int i = employee_index; i < dbcount - 1; i++) {
	  (*employees)[i] = (*employees)[i + 1];
	};
   

	memset(&(*employees)[dbcount - 1], 0, sizeof(struct employee_t));
	int safe_count = dbcount - 1;
	if (safe_count == 0){
		free(*employees);
		*employees = NULL;
		return STATUS_SUCCESS;

	};
	struct employee_t *temp = realloc(*employees, sizeof(struct employee_t) * (dbcount - 1));
	if (temp != NULL) {
		*employees = temp;
	};
	return STATUS_SUCCESS;
};

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
	if (write(fd, dbhdr, sizeof(struct dbheader_t)) == -1) {
		perror("write");
		return -1;
	};
	
	//Restoring endianness to host because the next function using the data will not expect it
    dbhdr->magic = ntohl(dbhdr->magic);
    dbhdr->version = ntohs(dbhdr->version);
    dbhdr->count = ntohs(dbhdr->count);
    dbhdr->filesize = ntohl(dbhdr->filesize);
	

	int i = 0;
	for (; i < realcount; i++){
		employees[i].hours = htonl(employees[i].hours);
		if (write(fd, &employees[i], sizeof(struct employee_t)) == -1) {
			perror("write");
			return -1;
		};
		//Back to host byte order
		employees[i].hours = ntohl(employees[i].hours);
	};
	if (ftruncate(fd, sizeof(struct dbheader_t) + (realcount * sizeof(struct employee_t))) == -1) {
		perror("ftruncate");
		return -1;
	}
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
	  printf("st_size - %ld, filesize - %u\n", dbfilestat.st_size, header->filesize);
	  printf("Database file size mismatch, possible corruption\n");
	  free(header);
	  return STATUS_ERROR;
	}

	*headerOut = header;
	return STATUS_SUCCESS;

}

int create_db_header(struct dbheader_t **headerOut) {
	
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



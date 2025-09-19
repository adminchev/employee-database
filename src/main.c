#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"

void print_usage(char *argv[]) {
	printf("Usage: %s -n -f <database file>\n", argv[0]);
	printf("\t -n - create new file\n");
	printf("\t -f - (required) path to file\n");
}

int main(int argc, char *argv[]) { 
	char *filepath = NULL;
	bool newfile = false;
  int dbfd = -1;
	int c = 0;
  struct dbheader_t *header = NULL;
  struct employee_t *employees = NULL;
  char *addstring = NULL;

	while (( c = getopt(argc, argv, "nf:a:")) != -1 ) {
		switch(c){
			case 'n':
				newfile = true;
				break;

			case 'f':
				filepath = optarg;
				break;

      case 'a':
        addstring = optarg;
        break;

			case '?':
				printf("Unknown option -%c.\n", c);
				break;

			default:
				return STATUS_ERROR;
		}
	}

	if (filepath == NULL) {
		printf("Filepath is a required argument.\n");
		print_usage(argv);
		return STATUS_SUCCESS;
	};

	printf("Newfile : %d\n", newfile);
	printf("Filepath: %s\n", filepath);
  
  if (newfile == true) {
    dbfd = create_db_file(filepath);
    
    if (dbfd == -1){
      printf("Failed to create file %s\n", filepath);
      return STATUS_ERROR;
    };
    
    if (create_db_header(dbfd, &header) == STATUS_ERROR) {
      printf("Failed to create database header\n");
      return STATUS_ERROR;
    };
  } else {

    dbfd = open_db_file(filepath);
    if (dbfd == -1) {
      printf("Could not open file\n");
      return STATUS_ERROR;
    };

    if (validate_db_header(dbfd, &header) == STATUS_ERROR) {
      printf("Failed to validate database header\n");
      return STATUS_ERROR;
    };
  
    if (read_employees(dbfd, header, &employees) != STATUS_SUCCESS) {
      printf("Something went wrong reading employee data\n");
      return STATUS_ERROR;
  }

  };

  if (addstring != NULL) {
    header->count++;
    employees = realloc(employees, header->count * (sizeof(struct employee_t)));
    add_employee(header, employees, addstring);
  };

  output_file(dbfd, header, employees);

  return 0;
}

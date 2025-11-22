#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>

#include "common.h"
#include "file.h"
#include "client.h"
#include "server.h"
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
	int ret = STATUS_ERROR;	
	while (( c = getopt(argc, argv, "nf:a:ld:c:")) != -1 ) {
		switch(c){
			case 'n':
				newfile = true;
				break;

			case 'f':
				filepath = optarg;
				break;

			case '?':
				printf("Unknown option -%c.\n", c);
				break;

			default:
				goto cleanup;
		};
	}

	if (filepath == NULL) {
		printf("Filepath is a required argument.\n");
		print_usage(argv);
		ret = STATUS_SUCCESS;
		goto cleanup;
	};

  
	if (newfile == true) {
		dbfd = create_db_file(filepath);
    
		if (dbfd == -1){
			printf("Failed to create file %s\n", filepath);
			goto cleanup;
		};
    
		if (create_db_header(&header) == STATUS_ERROR) {
			printf("Failed to create database header\n");
			goto cleanup;
		};

		if (output_file(dbfd, header, NULL) == STATUS_ERROR) {
			printf("Failed to write database header to file\n");
			goto cleanup;
		};

	} else {

		dbfd = open_db_file(filepath);
		if (dbfd == -1) {
			printf("Could not open file\n");
			goto cleanup;
		};

		if (validate_db_header(dbfd, &header) == STATUS_ERROR) {
			printf("Failed to validate database header\n");
			goto cleanup;
		};
  
		if (read_employees(dbfd, header, &employees) != STATUS_SUCCESS) {
			printf("Something went wrong reading employee data\n");
			goto cleanup;
		};

	};
	ret = server_loop(dbfd, header, employees); 

cleanup:
	free(employees);
	free(header);
	employees = NULL;
	header = NULL;
	if (dbfd != -1) {
		close(dbfd);
	};	

	return ret;
}

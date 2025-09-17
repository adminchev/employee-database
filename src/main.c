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
	int c = 0;

	while (( c = getopt(argc, argv, "nf:")) != -1 ) {
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
				return -1;
		}
	}

	if (filepath == NULL) {
		printf("Filepath is a required argument.\n");
		print_usage(argv);
		return 0;
	}

	printf("Newfile : %d\n", newfile);
	printf("Filepath: %s\n", filepath);
  
  if (newfile == true) {
    int fd = create_db_file(filepath);
    if (fd == -1){
      printf("Failed to create file %s\n", filepath);
      return STATUS_ERROR;
    }
  }
  return 0;
}

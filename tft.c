#include<stdio.h>
#include<stdlib.h>

static char **tbl_head = NULL;

int main(int argc, char **argv) {

	int i;
	FILE *file_to_parse;

	file_to_parse = fopen(argv[1], "r");


	if (!file_to_parse) {
		fprintf(stderr, "Couldn't open file %s, probably it doesn't exist\n", argv[1]);
		exit(EXIT_FAILURE);
	}

}

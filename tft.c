#include<stdio.h>
#include<stdlib.h>

enum ExitStatus {
	OOMEM = 1,
	NULLARG,
};

enum Bool {
	FALSE = 0,
	TRUE,
};

enum NodeType {
	NODE,       /* the node points to an array of Nodes */
	STRING,     /* points to a string, node does not any children nodes */
};

enum TokenType {
	ERROR = 0,
	EMPTY,
	KEY,        /* a string key in the table */
	VALUE,      /* a value that has a string key */
	IVALUE,     /* an indexed value that has a number index key */
};

struct Token;
struct Node;
struct Delm;

struct Delm {
	int isnode;
	union {
		char *delm;
		struct Node *node;
	};
	struct Delm *next;
};

/* */
struct Token {
	int type;
	char *start;
	char *end;
};

struct Node {
	int type;
	int ncnodes;
	char *index;
	union {
		char *string;
		struct Node *child;
	};
	struct Node *sibling;
};


// struct EndNode **get_end_nodes(
// 	char **delms, int ndelms, char leftchar, char rightchar, int *nnodes);
char *read_file(char *file, int *nread);
int get_delms(
	struct Delm **delimters, char *str, char *dtosearch, int *nmatch);

struct Node *create_node() {

	struct Node *node;

	node = malloc(sizeof(struct Node));

	if (!node) {
		fprintf(stderr, "Out of memmory, bailing out\n");
	}

	return node;
}

struct Delm *create_delm() {

	struct Delm *delm;

	delm = malloc(sizeof(struct Delm));

	if (!delm) {
		fprintf(stderr, "Out of memmory, bailing out\n");
	}

	return delm;
}

void dispose_node(struct Node *node) {

	if (!node) {
		return;
	}

	free(node);
}

void dispose_delm(struct Delm *dlem) {

	if (!dlem) {
		return;
	}

	free(dlem);
}

int get_delms(
	struct Delm **delimters, char *str, char *dtosearch, int *nmatch) {

	struct Delm *delms;
	struct Delm *curdelm = NULL;
	struct Delm *predelm = NULL;
	char *schar = dtosearch;
	int nm = 0;

	if (!str || !dtosearch || !delimters || !nmatch) {
		return NULLARG;
	}

	while (*str) {
		while (*schar) {
			if (*str == *schar) {

				curdelm = create_delm();

				curdelm->delm = str;
				curdelm->isnode = 0;
				nm++;

				if (predelm) {
					predelm->next = curdelm;
				} else {
					delms = curdelm;
				}

				predelm = curdelm;
				curdelm->next = NULL;
			}
			schar++;
		}
		str++;
		schar = dtosearch;
	}

	*delimters = delms;
	*nmatch = nm;

	return 0;

}

char *read_file(char *file, int *nread) {

	int read_chnr = 0;
	char *str;
	char ch;
	FILE *fp;

	if (!file) {
		return NULL;
	}

	fp = fopen(file, "r");

	if (!fp) {
		fprintf(stderr, "Couldn't open file %s, probably it doesn't exist\n", file);
		exit(EXIT_FAILURE);
	}

	str = malloc(sizeof(char) * BUFSIZ);

	if (!str) {
		fprintf(stderr, "Out of memory\n");
		return NULL;
	}

	while((ch = fgetc(fp)) != (char)EOF) {

		*(str + read_chnr) = ch;
		read_chnr++;

		if (read_chnr % BUFSIZ == 0) {
			str = realloc(str, sizeof(char) * (read_chnr + BUFSIZ));

			if (!str) {
				fprintf(stderr, "Out of memory\n");
				return NULL;
			}

		}

	}

	*(str + read_chnr) = '\0';

	fclose(fp);

	if (nread) {
		*nread = read_chnr;
	}

	return str;

}

void checking() {

	struct Node tbl;
	char hai[] = "lol\n\0";


	tbl.string = hai;

	printf("%s", tbl.string);

}

void get_token(
	struct Token *token, char *leftdelm, char *rightdelm) {

	char *tstart = leftdelm + 1;
	char *tend = rightdelm - 1;

	while (tstart != rightdelm) {
		switch (*tstart) {
			case '\n':
			case ' ':
			case '\t':
			default: break;
		}
		tstart++;
	}

	while (tend != leftdelm) {
		switch (*tend) {
			case '\n':
			case ' ':
			case '\t':
			default: break;
		}
		tend--;
	}

	/* catching empty tokens that are supposed to have someting */
	if ((*leftdelm != '{' && *rightdelm == '}')
		&& tstart == rightdelm || tend == leftdelm) {

		fprintf(stderr, "Missing token values\n");
		exit(EXIT_FAILURE);

	}

	if (*rightdelm == '=') {
		token->type = KEY;
	} else if (*leftdelm == '=') {
		token->type = VALUE;
	} else if (tstart == rightdelm || tend == leftdelm) {
		token->type = EMPTY;
	} else {
		token->type = IVALUE;
	}

	token->start = tstart;
	token->end = tend;

}

struct Node *make_node(struct Delm *lend, struct Delm *rend) {

	struct Node *node;
	struct Delm *curdelm = lend;
	struct Node *prev_child = NULL;
	char *curchar;
	int childnr = 0;

	node = create_node();

	while (curdelm != rend) {

		if (curdelm->isnode) {
			if (prev_child)
				prev_child->sibling = curdelm->node;
			else
				node->child = curdelm->node;
		} else {

		}


		curdelm = curdelm->next;

	}



}

int gen_tree(struct Node **node, struct Delm *delms, int ndelms) {

	int i;
	struct Delm *leftdelm = NULL;
	struct Delm *rightdelm = NULL;
	struct Node *parnode;
	struct Node *curnode;
	struct Node *eldnode;
	struct Delm *curdelm;

	if (!node || !delms || ndelms < 0) {
		return NULLARG;
	}

	parnode = create_node();

	for (curdelm = delms; curdelm; curdelm = curdelm->next) {

		if (curdelm->isnode == 0) {
			if (*(curdelm->delm) == '{') {
				leftdelm = curdelm;
			} else if (*(curdelm->delm) == '}' && leftdelm) {
				rightdelm = curdelm;
			}
		}

		if (leftdelm && rightdelm) {
			curnode = make_node(leftdelm, rightdelm);
		} else if (leftdelm) {
			fprintf(stderr, "Missing a }\n");
		} else if (rightdelm) {
			fprintf(stderr, "Missing a {\n");
		} else {
		}
	}

}

int parse_string(char *str, int len) {

	int no_delms;
	int no_end_nodes;
	int i;
	struct Delm *delms;
	struct Node *curnode;

	char dtosearch[] = "={},\0";

	get_delms(&delms, str, dtosearch, &no_delms);


	// end_nodes = get_end_nodes(delms, no_delms, &no_end_nodes);

	while (delms) {
		printf("%c\n", *(delms->delm));
		delms = delms->next;
	}

	printf("%s\n", str);
	printf("%d\n", len);
	printf("%d\n", no_delms);

}


int main(int argc, char **argv) {

	int i;
	int nread;
	char *str;

	str = read_file(argv[1], &nread);

	parse_string(str, nread);


	checking();

}

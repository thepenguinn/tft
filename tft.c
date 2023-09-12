#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>

enum DelmType {
	DELM_NODE,
	DELM_CHAR,
};

enum ErrorType {
	ERROR_NONE   = 0b0000,
	ERROR_KEY    = 0b0001,
	ERROR_VALUE  = 0b0010,
	ERROR_IVALUE = 0b0100,
};

enum NodeType {
	NODE_CHILD,      /* the node points to an array of Nodes */
	NODE_STRING,     /* points to a string, node does not any children nodes */
};

enum TokenType {
	TOKEN_NULL = 0,
	TOKEN_KEY,        /* a string key in the table */
	TOKEN_VALUE,      /* a value that has a string key */
	TOKEN_IVALUE,     /* an indexed value that has a number index key */
};

enum KeyType {
	KEY_NUM,
	KEY_STR,
};

struct Token;
struct Node;
struct Delm;

struct Delm {
	enum DelmType dtype;
	union {
		char *delm;
		struct Node *node;
	};
	struct Delm *next;
};

struct Token {
	enum TokenType ttype;
	enum ErrorType etype;
	char *start;
	char *end;
};

struct Node {
	enum NodeType ntype;
	enum KeyType ktype;
	enum ErrorType etype;
	int childnr;
	union {
		int keynum;
		char *keystr;
	};
	union {
		char *valstr;
		struct Node *child;
	};
	struct Node *sibling;
};

static char char_minus[] = "─\0";
static char char_pipe[]  = "│\0";
static char char_tee[]   = "├\0";
static char char_elbow[] = "└\0";

struct Node *create_node();
struct Delm *create_delm();
void dispose_node(struct Node *node);
void dispose_delm(struct Delm *dlem);

static int calc_strlen(char *start, char *end);
static char *tkn_tostr(struct Token *tkn);
static void gen_rootkey(struct Token *tkn, char *left, char *right);
static void gen_token( struct Token *tkn, char *leftdelm, char *rightdelm);

struct Node *make_node(struct Delm *leftdelm, struct Delm *rightdelm);
int capture_nodes(struct Delm *delms, int ndelms);
struct Node *get_rootnode(char *str, struct Delm *delms);
int get_delms(struct Delm **delimters, char *str, int *nmatch);
struct Node *parse_string(char *str, int len);

char *read_file(char *file, int *nread);

void draw_tree(int depth, struct Node *node);

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

static int calc_strlen(char *start, char *end) {

	// TODO: Make this a macro
	int len;

	len = (int)(end - start);

	if (len < 0)
		len = ~len + 1;

	return len + 1;

}

static char *tkn_tostr(struct Token *tkn) {

	char *dst;
	char *tmp;
	char *src = tkn->start;
	int len;

	if (!src) {
		return NULL;
	}

	len = calc_strlen(tkn->start, tkn->end);

	tmp = dst = malloc(sizeof(char) * (len + 1));

	while (len) {
		*tmp = *src;
		tmp++;
		src++;
		len--;
	}

	*tmp = '\0';

	return dst;

}

static void gen_rootkey(struct Token *tkn, char *left, char *right) {

	char local[] = "local\0";
	char *lch = local;
	char *tmp;
	int i = 0;

	while (*left) {
		if  (*left != '\n' && *left != ' ' && *left != '\t') {
			break;
		}
		left++;
	}

	tmp = left;

	while (*lch && *tmp && *tmp == *lch) {
		i++;
		tmp++;
		lch++;
	}

	if (i == 5) {
		left = tmp;
		while (*left && left != right) {
			if  (*left != '\n' && *left != ' ' && *left != '\t') {
				break;
			}
			left++;
		}

	}

	while (*right && right != left) {
		if  (*right != '\n' && *right != ' ' && *right != '\t') {
			break;
		}
		right--;
	}

	tkn->ttype = TOKEN_KEY;

	if (left == right) {

		tkn->etype = ERROR_KEY;
		tkn->start = NULL;

	} else {

		tkn->etype = ERROR_NONE;
		tkn->start = left;
		tkn->end = right;

	}

}

static void gen_token( struct Token *tkn, char *leftdelm, char *rightdelm) {

	char *tstart = leftdelm + 1;
	char *tend = rightdelm - 1;

	while (tstart != rightdelm) {
		if  (*tstart != '\n' && *tstart != ' ' && *tstart != '\t') {
			break;
		}
		tstart++;
	}

	while (tend != leftdelm) {
		if  (*tend != '\n' && *tend != ' ' && *tend != '\t') {
			break;
		}
		tend--;
	}

	if (*leftdelm == '=') {

		if (*rightdelm == '=') {
			fprintf(stderr, "Expected the value token to end with a ',' or a '}'\n");
			exit(EXIT_FAILURE);
		}

		tkn->ttype = TOKEN_VALUE;
		if (tstart == rightdelm) {
			tstart = NULL;
			tkn->etype = ERROR_VALUE;
		} else {
			tkn->etype = ERROR_NONE;
		}

	} else if (*rightdelm == '=') {

		tkn->ttype = TOKEN_KEY;
		if (tstart == rightdelm) {
			tstart = NULL;
			tkn->etype = ERROR_KEY;
		} else {
			tkn->etype = ERROR_NONE;
		}

	} else if (*rightdelm == '}') {

		if (*leftdelm == '{') {
			if (tstart == rightdelm) {
				tkn->ttype = TOKEN_NULL;
			} else {
				tkn->ttype = TOKEN_IVALUE;
			}
		} else if (*leftdelm == ',') {
			if (tstart == rightdelm) {
				tkn->ttype = TOKEN_NULL;
			} else {
				tkn->ttype = TOKEN_IVALUE;
			}
		}

		tkn->etype = ERROR_NONE;

	} else {

		tkn->ttype = TOKEN_IVALUE;
		if (tstart == rightdelm) {
			tstart = NULL;
			tkn->etype = ERROR_IVALUE;
		} else {
			tkn->etype = ERROR_NONE;
		}


	}

	tkn->start = tstart;
	tkn->end = tend;

}

struct Node *make_node(struct Delm *leftdelm, struct Delm *rightdelm) {

	struct Delm *curdelm = leftdelm;
	struct Delm *nexdelm;
	struct Node *parnode;
	struct Node *prenode = NULL;
	struct Node *curnode = NULL;;
	struct Token tkn;
	int childnr = 0;
	int idx = 0;

	tkn.ttype = TOKEN_NULL;

	parnode = create_node();
	parnode->sibling = NULL;
	parnode->ntype = NODE_CHILD;
	parnode->etype = ERROR_NONE;
	parnode->child = NULL;

	while (curdelm != rightdelm) {

		nexdelm = curdelm->next;

		if (nexdelm->dtype == DELM_NODE) {

			prenode = curnode;
			curnode = nexdelm->node;
			childnr++;

			switch (tkn.ttype) {
				case TOKEN_KEY:
					curnode->ktype = KEY_STR;
					curnode->keystr = tkn_tostr(&tkn);
					curnode->etype = tkn.etype;
					break;
				default:
					curnode->ktype = KEY_NUM;
					curnode->keynum = ++idx;
			}

			if (prenode) {
				prenode->sibling = curnode;
			} else {
				parnode->child = curnode;
			}

			if (nexdelm->next->dtype == DELM_CHAR) {
				switch (*(nexdelm->next->delm)) {
					case ',':
						if (curdelm != leftdelm)
							dispose_delm(curdelm);
						curdelm = nexdelm;

						break;
					case '}':
						break;
					default:
						fprintf(stderr, "Missing a comma\n");
						exit(EXIT_FAILURE);
				}
			} else {
				fprintf(stderr, "Missing a comma\n");
				exit(EXIT_FAILURE);
			}

			tkn.ttype = TOKEN_NULL;

		} else if (nexdelm->dtype == DELM_CHAR) {

			if (curdelm->dtype == DELM_CHAR) {

				if (tkn.ttype == TOKEN_KEY) {

					prenode = curnode;
					curnode = create_node();
					curnode->etype = tkn.etype;
					childnr++;

					curnode->ktype = KEY_STR;
					curnode->keystr = tkn_tostr(&tkn);
				}

				gen_token(&tkn, curdelm->delm, nexdelm->delm);

				switch (tkn.ttype) {
					case TOKEN_IVALUE:
						prenode = curnode;
						curnode = create_node();
						curnode->etype = tkn.etype;
						childnr++;

						curnode->ktype = KEY_NUM;
						curnode->keynum = ++idx;

					case TOKEN_VALUE:
						curnode->ntype = NODE_STRING;
						curnode->valstr = tkn_tostr(&tkn);
						curnode->etype = curnode->etype | tkn.etype;

						if (prenode) {
							prenode->sibling = curnode;
						} else {
							parnode->child = curnode;
						}

						break;
					default:
						break;
				}


			} else {
				fprintf(stderr, "Thats supposed to be a delimiter.\n");
				exit(EXIT_FAILURE);
			}

		}

		if (curdelm != leftdelm)
			dispose_delm(curdelm);
		curdelm = curdelm->next;

	}

	if (curnode)
		curnode->sibling = NULL;

	leftdelm->dtype = DELM_NODE;
	leftdelm->node = parnode;
	leftdelm->next = rightdelm->next;
	dispose_delm(rightdelm);

}

int capture_nodes(struct Delm *delms, int ndelms) {

	int i;
	struct Delm *leftdelm = NULL;
	struct Delm *rightdelm = NULL;
	struct Delm *curdelm;
	int capnodes = 0;

	if (!delms || ndelms < 0) {
		return 1;
	}

	while (1) {
		for (curdelm = delms; curdelm; curdelm = curdelm->next) {

			if (curdelm->dtype == DELM_CHAR) {
				if (*(curdelm->delm) == '{') {
					leftdelm = curdelm;
				} else if (*(curdelm->delm) == '}' && leftdelm) {
					rightdelm = curdelm;
				}

			}

			if (leftdelm && rightdelm) {
				make_node(leftdelm, rightdelm);
				leftdelm = rightdelm = NULL;
				capnodes++;
			}

		}

		if (!capnodes) {
			if (leftdelm) {
				fprintf(stderr, "Probabily missing a brace\n");
				exit(EXIT_FAILURE);
			}
			return 0;
		}
		capnodes = 0;
		curdelm = delms;
	}

}

struct Node *get_rootnode(char *str, struct Delm *delms) {

	struct Node *rnode = NULL;
	struct Token tkn;

	switch (delms->dtype) {
		case DELM_CHAR:
			if (*(delms->delm) == '=') {

				if (delms->next && delms->next->dtype == DELM_NODE) {
					rnode = delms->next->node;

					gen_rootkey(&tkn, str, delms->delm);

					rnode->etype = tkn.etype;
					rnode->ktype = KEY_STR;
					rnode->keystr = tkn_tostr(&tkn);

				} else {
					fprintf(stderr, "Next delimiter is not a node.\n");
					exit(EXIT_FAILURE);
				}
			} else {
				fprintf(stderr, "Syntax error\n");
				exit(EXIT_FAILURE);
			}

			break;
		case DELM_NODE:
			rnode = delms->node;
			rnode->sibling = NULL;
			rnode->etype = ERROR_NONE;
			rnode->ktype = KEY_NUM;
			rnode->keynum = 1;
			break;
		default:
			fprintf(stderr, "Syntax error\n");
			exit(EXIT_FAILURE);

	}

	return rnode;
}


int get_delms( struct Delm **delimters, char *str, int *nmatch) {

	struct Delm *delms;
	struct Delm *curdelm = NULL;
	struct Delm *predelm = NULL;
	char dtosearch[] = "={},\0";
	char *schar = dtosearch;
	int nm = 0;

	if (!delimters || !str || !nmatch) {
		return 1;
	}

	while (*str) {
		while (*schar) {
			if (*str == *schar) {

				curdelm = create_delm();
				if (predelm) {
					predelm->next = curdelm;
				} else {
					delms = curdelm;
				}

				curdelm->delm = str;
				curdelm->dtype= DELM_CHAR;
				curdelm->next = NULL;
				nm++;

				predelm = curdelm;

				break;
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

struct Node *parse_string(char *str, int len) {

	int no_delms;
	int no_end_nodes;
	struct Delm *delms;
	struct Delm *tmpdelm;
	struct Node *rnode;

	get_delms(&delms, str, &no_delms);

	if (!no_delms) {
		fprintf(stderr, "Couldn't find any delimiters\n");
		exit(EXIT_FAILURE);
	}

	capture_nodes(delms, no_delms);

	rnode = get_rootnode(str, delms);

	while (delms) {
		tmpdelm = delms->next;
		dispose_delm(delms);
		delms = tmpdelm;
	}

	return rnode;

}

char *read_file(char *file, int *nread) {

	int read_chnr = 0;
	char *str;
	char ch;
	FILE *fp;
	struct stat st;

	if (!file) {
		return NULL;
	}

	if (stat(file, &st)) {
		fprintf(stderr, "Couldn't open file %s, probably it doesn't exist.\n", file);
		exit(EXIT_FAILURE);
	}

	if (S_ISDIR(st.st_mode)) {
		fprintf(stderr, "%s is a directory.\n", file);
		exit(EXIT_FAILURE);
	}

	fp = fopen(file, "r");

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

void draw_tree(int depth, struct Node *node) {

	int i;

	while (node) {

		if (node->ntype == NODE_CHILD) {
			for (i = 0; i < depth; i++) {
				printf("    ");
			}
			printf("NODE_CHILD\n");
			draw_tree(depth + 1, node->child);
		} else if (node->ntype == NODE_STRING) {
			for (i = 0; i < depth; i++) {
				printf("    ");
			}
			printf("%s\n", node->valstr);
		} else {
			fprintf(stderr, "That not supposed to be happening\n");
		}

		node = node->sibling;
	}

}


int main(int argc, char **argv) {

	int nread;
	char *str;
	struct Node *rnode;

	if (argc < 2) {
		fprintf(stderr, "File name not specified.\n");
		exit(EXIT_FAILURE);
	}

	str = read_file(argv[1], &nread);

	rnode = parse_string(str, nread);

	free(str);

	draw_tree(0, rnode);

	// checking();

}

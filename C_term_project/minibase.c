/*
C term project
Yijun Cao[1352872]
<classone2010@gmail.com>
key-value memory storage, implemented B+ tree
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define VERSION "0.1"

typedef enum PARSE_LINE_ERROR_FLAG
{
	SUCCESS,
	END_OF_LINE,
	END_OF_FILE,
	PARSE_ERROR,
	FILE_ERROR
} PARSE_LINE_ERROR_FLAG;

// #define MAX_KEYS_IN_NODE 1024
// #define MAX_RECORDS_IN_LEAF 64
// MAX_KEYS_IN_NODE should NOT be 1. binary tree are not supported. it's a pure B+tree.
#define MAX_KEYS_IN_NODE 1024
#define MAX_POINTERS_IN_NODE (MAX_KEYS_IN_NODE + 1)
#define MAX_RECORDS_IN_LEAF 64
#define u_int unsigned int

typedef struct Node
{
	char   **keys;       //keys in current Node, i use char *as key.
	u_int key_num;    //the number of keys/records in current Node
	struct Node  *parent;     //parent Node
	void   **pointers;   //pointers to children nodes if it's a Node,pointers to Records if it's a leaf Node.
	bool   is_leaf;
	struct Node  *right_node;
} Node;

// character which separate the key and value

typedef struct Record
{
	char *key;
	char *value;
} Record;
#include "minibase.h"

char SEPATATOR = ',';

void print_record(const Record *record){
	printf("record:%s%c%s\n", record->key, SEPATATOR, record->value);
}

Record *make_record(char *key, char *value) {
	Record *new_record = (Record *)malloc(sizeof(Record));
	new_record->key = key;
	new_record->value = value;
	return new_record;
}

// key and value actually malloced together, separated by '\0'
// and the memory space of record is malloced by another function.
void free_record(Record *record){
	free(record->key);
	free(record);
}

Node *make_node(void){
	return _make_node(MAX_KEYS_IN_NODE, false);
}

Node *make_leaf(void){
	Node *leaf = _make_node(MAX_RECORDS_IN_LEAF, true);
	return leaf;
}

Node *_make_node(u_int max_key_num, bool is_leaf) {
	Node *new_node;
	new_node = (Node *)malloc(sizeof(Node));
	// leaf_node have MAX_RECORDS_IN_LEAF keys and pointers
	// inner node have MAX_KEYS_IN_NODE keys, and MAX_POINTERS_IN_NODE pointers.
	new_node->keys = (char **)malloc(max_key_num * sizeof(char **));
	new_node->pointers = (void **)malloc((is_leaf ? max_key_num:(max_key_num + 1)) *sizeof(void **));
	new_node->is_leaf = is_leaf;
	new_node->key_num = 0;
	new_node->parent = NULL;
	new_node->right_node = NULL;
	return new_node;
}

// like memcpy, but do it backward. for the dst and src are overlapping.
// ====src++++dst____
// ====src====src++++		// result like this.
void shift_backward(void *dst, const void *src, u_int length){
	while(length--)
		*((char *)dst + length) = *((char *)src + length);
}

void pointer_shift(void **dst, const void **src, u_int length){
	while(length--)
		*((char **)dst + length) = *((char **)src + length);
}

void pointer_copy(void **dst, const void **src, u_int length){
	while(length--)
		*(char **)dst++ = *(char **)src++;
}

// =.= , term project require us not to use string.h.
// what a bad idea, isn't it.
void memcpy(void *dst, const void *src, u_int length){
	while(length--)
		*(char *)dst++ = *(char *)src++;
}

int strcmp(char const *str1, char const *str2){
	while (*str1 != '\0' && *str2 != '\0' && *str1 == *str2){
		str1++;
		str2++;
	}
	return *str1 - *str2;
}

/*special binary search, search Key in [Key0, Keyn]
 *return 0 if Key < Key0
 *return i if key = keyi
 *return i+1 if Keyi <= Key <Key(i+1)
 *rewrite various version of b_search, to reduce function calls
 */
int b_search_subtree(void *base, int total, char *target, int(*cmp)(char const *, char const *)){
	int head = 0;
	int tail = total - 1;
	int mid,result;
	while (head <= tail){
		mid = (head + tail)/2;
		result = cmp(*((char **)base + mid), target);
		if (result > 0)
		{
			tail = mid - 1;
		}
		else if (result < 0)
		{
			head = mid + 1;
		}else{
			return mid + 1;               // found and return right subtree index
		}
	}
	return (result > 0) ? mid : mid + 1;  // return right subtree index
}

// return whether have a exact match 
// index of exact match subscript or 
bool b_search_key(u_int *index, void *base, u_int total, char *target, int(*cmp)(char const *, char const *)){
	int head = 0;
	int tail = total - 1;
	int mid, result;
	while (head <= tail){
		mid = (head + tail)/2;
		result = cmp(*((char **)base + mid), target);
		if (result > 0)
		{
			tail = mid - 1;
		}
		else if (result < 0)
		{
			head = mid + 1;
		}else{
			*index = mid;
			return true;
		}
	}
	*index = (result > 0) ? mid : (mid + 1);
	return false;
}

Node *find_leaf(Node *root, char *key) {
	while(!root->is_leaf){
		int i = b_search_subtree(root->keys, root->key_num, key, strcmp);
		root = (Node *)root->pointers[i];
	}
	return root;
}

//return 0 if not found
Record *find(Node *root, char *key) {
	Node *leaf = find_leaf(root, key);
	u_int index;
	if (b_search_key(&index, leaf->keys, leaf->key_num, key, strcmp))
	{
		return (Record *)leaf->pointers[index];
	};
	return 0;
}

Node *init_new_tree(char *key, Record *Record) {
	Node *root = make_leaf();
	root->keys[0] = key;
	root->pointers[0] = Record;
	root->pointers[MAX_RECORDS_IN_LEAF - 1] = NULL;
	root->parent = NULL;
	root->key_num++;
	return root;
}

Node *insert(Node *root, char *key, char *value) {
	return insert_record(root, make_record(key, value));
}

Node *insert_record(Node *root, Record *new_record) {

	Node *leaf;

	/*Case: the tree does not exist yet.
	 *Start a new tree.
	 */
	if (root == NULL) 
		return init_new_tree(new_record->key, new_record);

	/*Case: the tree already exists.
	 *(Rest of function body.)
	 */
	leaf = find_leaf(root, new_record->key);

	return insert_into_leaf(root, leaf, new_record->key, new_record);
	// /*Case: leaf has room for key and new_record.
	//  */

	// if (leaf->key_num < MAX_RECORDS_IN_LEAF) {
	// 	leaf = 
	// 	return root;
	// }
	// //, or borrow space from neighbour
	// // try_borrow_from_neighbour();

	// /*Case:  leaf must be split.
	//  */

	// return insert_into_leaf_and_split(root, leaf, new_record->key, new_record);
}

void _write_into_leaf(Node *leaf, char *key, u_int insertion_point, void *record){
	leaf->keys[insertion_point] = key;
	leaf->pointers[insertion_point] = record;
}

void _add_into_leaf(Node *leaf, char *key, u_int insertion_point, void *record){
	leaf->keys[insertion_point] = key;
	leaf->pointers[insertion_point] = record;
	leaf->key_num++;
}

void _write_into_node(Node *node, char *key, u_int insertion_point, void *left_node, void *right_node){
	node->keys[insertion_point] = key;
	node->pointers[insertion_point] = left_node;
	node->pointers[insertion_point + 1] = right_node;
}

void _add_into_node(Node *node, char *key, u_int insertion_point, void *left_node, void *right_node){
	node->keys[insertion_point] = key;
	node->pointers[insertion_point] = left_node;
	node->pointers[insertion_point + 1] = right_node;
	node->key_num++;
}

Node *insert_into_leaf(Node *root, Node *leaf, char *key, Record *pointer) {
	u_int insertion_point;
	//no exact match
	if (!b_search_key(&insertion_point, leaf->keys, leaf->key_num, key, strcmp)){
		if (leaf->key_num == MAX_RECORDS_IN_LEAF){ // Split needed.
			return insert_into_leaf_and_split(root, leaf, key, insertion_point, pointer);
		}
		pointer_shift(leaf->keys + insertion_point + 1, leaf->keys + insertion_point, leaf->key_num - insertion_point);
		pointer_shift(leaf->pointers + insertion_point + 1, leaf->pointers + insertion_point, leaf->key_num - insertion_point);
		_add_into_leaf(leaf, key, insertion_point, pointer);
	}else{
		//or just cover the origin record for exact match.
		free_record((Record *)leaf->pointers[insertion_point]);
		_write_into_leaf(leaf, key, insertion_point, pointer);
	}
	return root;
}

u_int cut( u_int length ) {
	if (length % 2 == 0)
		return length/2;
	else
		return length/2 + 1;
}

void reset_parent(Node *node){
	for (u_int i = 0; i <= node->key_num; ++i)
	{
		((Node *)(node->pointers[i]))->parent = node;
	}
}

// a full leaf, which need to split.
Node *insert_into_leaf_and_split(Node *root, Node *leaf, char *key, u_int insertion_point, Record *record){
	char *extra_key;
	void *extra_pointer;
	u_int old_rec_num;
	u_int new_rec_num;
	// move elements for insertion.
	if (insertion_point < MAX_RECORDS_IN_LEAF){
		extra_key     = leaf->keys[leaf->key_num - 1];
		extra_pointer = leaf->pointers[leaf->key_num - 1];
		pointer_shift(leaf->keys     + insertion_point + 1, leaf->keys + insertion_point, 	  MAX_RECORDS_IN_LEAF - insertion_point - 1);
		pointer_shift(leaf->pointers + insertion_point + 1, leaf->pointers + insertion_point, MAX_RECORDS_IN_LEAF - insertion_point - 1);
		_write_into_leaf(leaf, key, insertion_point, record);
	}else{
		extra_key     = key;
		extra_pointer = record;
	}
	// cut it in half
	old_rec_num = leaf->key_num = cut(leaf->key_num  + 1);
	new_rec_num = MAX_RECORDS_IN_LEAF != 1 ? MAX_RECORDS_IN_LEAF - old_rec_num + 1: 1;
	// a new leaf to hold the rest 
	Node *new_leaf = make_leaf();
	pointer_copy(new_leaf->keys, leaf->keys + old_rec_num, (MAX_RECORDS_IN_LEAF - old_rec_num));
	pointer_copy(new_leaf->pointers, leaf->pointers + old_rec_num, (MAX_RECORDS_IN_LEAF - old_rec_num));
	// and the extra one 
	new_leaf->keys[new_rec_num - 1] = extra_key;
	new_leaf->pointers[new_rec_num - 1] = extra_pointer;
	new_leaf->key_num = new_rec_num;
	new_leaf->right_node = leaf->right_node;
	leaf->right_node = new_leaf;
	return inesrt_into_parent(root, leaf, new_leaf->keys[0], new_leaf);
}

Node *inesrt_into_parent(Node *root, Node *left_node, char *key, Node *right_node){
	Node *parent;
	parent = left_node->parent;
	// need a new root, eg grow.
	if (parent == NULL){
		root = make_node();
		_add_into_node(root, key, 0, left_node, right_node);
		left_node->parent = right_node->parent = root;
		return root;
	}
	right_node->parent = left_node->parent;
	// get key index
	u_int index = 0;
	while (index <= parent->key_num && parent->pointers[index] != left_node)
		index++;
	// happy case, engouh room for new node
	if (parent->key_num < MAX_KEYS_IN_NODE){
		pointer_shift(parent->keys + index + 1, parent->keys + index, parent->key_num - index);
		pointer_shift(parent->pointers + index + 2, parent->pointers + index + 1, parent->key_num - index);
		_add_into_node(parent, key, index, left_node, right_node);
		
		return root;
	}
	// parent node split needed.
	return insert_into_node_and_split(root, parent, key, index, left_node, right_node);
}

// a full ndoe which need to split
Node *insert_into_node_and_split(Node *root, Node *old_node, char *key, u_int index, Node *left_node, Node *right_node){
	char *extra_key;
	void *extra_pointer;
	u_int old_node_key_num;
	u_int new_node_key_num;
	// move elements for insertion.
	if (MAX_KEYS_IN_NODE > index){
		extra_key = old_node->keys[old_node->key_num - 1];
		extra_pointer = old_node->pointers[old_node->key_num];
		pointer_shift(old_node->keys + index + 1, old_node->keys + index, MAX_KEYS_IN_NODE - index - 1);
		pointer_shift(old_node->pointers + index + 2, old_node->pointers + index + 1, MAX_POINTERS_IN_NODE - index - 1);
		_write_into_node(old_node, key, index, left_node, right_node);
	}else{
		extra_key = key;
		extra_pointer = right_node;
	}
	old_node_key_num = old_node->key_num = cut(old_node->key_num);
	new_node_key_num = MAX_KEYS_IN_NODE != 1 ? MAX_KEYS_IN_NODE - old_node_key_num: 1;
	Node *new_node = make_node();
	new_node->right_node = old_node->right_node;
	old_node->right_node = new_node;
	pointer_copy(new_node->keys, old_node->keys + old_node_key_num + 1, MAX_KEYS_IN_NODE != 1 ? new_node_key_num - 1 : 0);
	pointer_copy(new_node->pointers, old_node->pointers + old_node_key_num + 1, MAX_POINTERS_IN_NODE - old_node_key_num);
	new_node->keys[new_node_key_num -1] = extra_key;
	new_node->pointers[new_node_key_num] = extra_pointer;
	new_node->key_num = new_node_key_num;
	reset_parent(new_node);
	return inesrt_into_parent(root, old_node, new_node->keys[0], new_node);
}

void print_welcome(){
	printf("in memory key-value mini database implemented b plus tree\n"
			"version %s\n", VERSION);
}

void print_help(){
	print_welcome();
	printf( "h                      print this help\n"
			"i <key>%c<value>  insert a record\n"
			"q                      quit\n"
			"and file options:\n"
			"-i data.txt\n", SEPATATOR);
}

void print_node(Node *node){
	u_int i = 0;
	printf("|%p:[%u] ", node, node->key_num);
	while (i < node->key_num){
		printf("%p,", node->pointers[i]);
		printf("%s,", node->keys[i]);
		i++;
	}
	printf("%p,", node->pointers[i]);
	printf("| # ");
}

void print_layer(Node *left_node){
	do{
		print_node(left_node);
		left_node = left_node->right_node;
	}
	while (left_node != NULL);
	printf("\n");
}

void print_leaf(Node *leaf_node){
	Record **record = (Record **)leaf_node->pointers;
	u_int i = leaf_node->key_num;
	printf("|%p:", leaf_node);
	do{
		printf(" %s,%s |", (*record)->key, (*record)->value);
		record++;
		i--;
	}
	while (i);
}

void print_leafs(Node *leaf_node){
	do{
		print_leaf(leaf_node);
		printf("#");
		leaf_node = leaf_node->right_node;
	}
	while (leaf_node != NULL);
	printf("\n");
}

void print_tree(Node *root){
	if (root == NULL){
		printf("empty tree\n");
		return;
	}
	while (!root->is_leaf){
		print_layer(root);
		root = (Node *)root->pointers[0];
	}
	print_leafs(root);
	printf("\n");
}

//msg should be a malloc memory space
void warn(char *msg){
	printf("warning: %s\n", msg);
	free(msg);
}

void fatal(const char *msg){
	printf("%s\n", msg);
	exit(1);
}

FILE *ensure_open_file(char const *filename){
	FILE *file;
	if ((file = fopen(filename, "r")) == NULL){
		char *buff = (char *)malloc(128);
		snprintf(buff, 128, "open file %s failed, file may not exist or permission denied.", filename);
		fatal(buff);
	}
	return file;
}

char *get_line(FILE *file){
	char ch;
	u_int buff_length = 16;
	u_int line_length = 0; 
	char *buff = (char *)malloc(buff_length);
	while(EOF != (ch=fgetc(file))){
		if (ch == '\n')
			break;
		buff[line_length] = ch;
		line_length++;
		if (line_length == buff_length - 1){
			buff_length *= 2;
			buff = realloc(buff, buff_length);
		}
	}
	buff = realloc(buff, line_length + 1);
	buff[line_length] = '\0';
	return buff;
}

Record *parse_line(FILE *file, PARSE_LINE_ERROR_FLAG *error){
	int buff_length = 16;
	char *pointer = (char *)malloc(buff_length);
	char *buff = pointer;
	u_int line_length = 0;
	char ch;

	bool comma_found = false;
	u_int comma = 0; 		//buff[comma-1] == SEPATATOR
	*error = SUCCESS;
	while(*error == SUCCESS){
		ch = fgetc(file);
		switch (ch) {
			case '\r':
				break;
			case '\n':
				*error = END_OF_LINE;
				break;
			case EOF:{
				if (ferror(file) != 0){
					*error = FILE_ERROR;
					clearerr(file);
				}else{
					*error = END_OF_FILE;
				}
				break;
			}
			default:
				if (!comma_found){
					comma++;
					if (ch==SEPATATOR){
					ch = '\0';       //replace SEPATATOR with '\0'
					comma_found = true;
					comma--;
					}
				}
				line_length++;
				*(buff++) = ch;
				if (line_length == buff_length - 1){
					buff_length *= 2;
					pointer =  (char *)realloc(pointer, buff_length);
					buff = pointer + (line_length);
				}
		}
	}
	if (*error != END_OF_FILE && ((comma > line_length) || (comma == line_length) || (comma == 0))){
		*error = PARSE_ERROR;
	}
	comma++;
	*(buff++) = '\0';
	pointer = (char *)realloc(pointer, buff_length);
	switch (*error) {
		case END_OF_LINE:
			*error = SUCCESS;
			return make_record(pointer, (pointer + comma));
		case PARSE_ERROR:
			return (Record *)pointer;
		case END_OF_FILE:
			return make_record(pointer, (pointer + comma));
		case FILE_ERROR:{
			while (EOF != (ch = fgetc(file)))
				if (ch == '\n') break;
			return (Record *)pointer;
		}
	}
}

Node *import_data(char const *filename, Node *root, u_int *error_count, u_int *record_count){
	FILE *file = ensure_open_file(filename);
	PARSE_LINE_ERROR_FLAG error = SUCCESS;
	Record *record;
	*error_count = 0;
	*record_count= 0;
	printf("Note: importing from %s\n", filename);
	while(error != END_OF_FILE){
		record = parse_line(file, &error);
		switch (error){
			case PARSE_ERROR:{
				char *buff = (char *)malloc(128);
				snprintf(buff, 128, "Parse Error: %s", (char *)record);
				warn(buff);
				(*error_count)++;
				break;
			}
			case FILE_ERROR:{
				char *buff = (char *)malloc(128);
				snprintf(buff, 128, "FILE Error: %s", (char *)record);
				warn(buff);
				(*error_count)++;
				break;
			}
			case SUCCESS:
			case END_OF_FILE:
				(*record_count)++;
				// print_record(record);
				root = insert_record(root, record);
				// print_tree(root);
				break;
		}
	}
	return root;
}

int std_arg_parser(char *str, char **argv_p[]){
	int argc = 1;
	char **argv = (char **)malloc(6 * sizeof(char *));
	*argv_p = argv;
	*argv++;        // we don't have argv[0], the executable path.
	*argv++ = str;
	while (*str != '\0') {
		if (*str == ' ') {
			*str++ = '\0';
			*argv++ = str;
			argc++;
			while (*str == ' ') str++;
		}
		str++;
	}

	return argc;
}

Node *stdin_mode(int argc, char const *argv[]){
	clock_t tImportStart;
	u_int error_count = 0;
	u_int record_count= 0;
	Node *root = NULL;
	if (*argv[1] != '-')
		fatal("need a option");
	switch (*(argv[1]+1)) {
		case 'h':
			print_help();
			break;
		case 'q':
			printf("bye\n");
			return;
		case 'c':
			if (argc < 2)
				fatal("-c need a file to create table");
			//create_table(argv[2]);
			break;
		case 'i':
			if (argc < 2)
				fatal("-i need a file to import data");
			tImportStart = clock();
			root = import_data(argv[2], root, &error_count, &record_count);
			printf("Note: %u records imported within %.2fs with %d parse error.\n", record_count,
							 (double)(clock() - tImportStart)/CLOCKS_PER_SEC, error_count);
			// interactive_mode(root);
			break;
		case 's':
			if (argc < 2)
				fatal("-s need a file to get the select query");
			//select_record(argv[2]);
			break;
		case 'u':
			if (argc < 2)
				fatal("-u need a file to get the update query");
			//update_data(argv[2]);
			break;
		case 'd':
			if (argc < 2)
				fatal("-d need a file to get the delete query");
			//delete_data(argv[2]);
			break;
	}
	return root;
}

void interactive_mode(Node *root){
	char command;
	PARSE_LINE_ERROR_FLAG error = SUCCESS;
	Record *record;
	clock_t tStart;
	printf("> ");
	while (EOF!= (command = fgetc(stdin))) {
		// printf("into while\n");
		switch (command) {
			case 'h':
				print_help();
				break;
			case 'q':
				printf("bye\n");
				return;
			case 'i':
				fgetc(stdin);   //jump a space
				record = parse_line(stdin, &error);
				if (error!=SUCCESS){
					printf("parse error\n"
						"key and value should be separated by a SEPATATOR. for example:\n"
						"name%cpasswordhash\n", SEPATATOR);
				}else{
					tStart = clock();
					root = insert_record(root, record);
					print_record(record);
					printf("inserted within %.2fs.\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
				}
				break;
			case 's':{
				if (!root){
					printf("need import some records first\n");
				}else{
					tStart = clock();
					fgetc(stdin);   //jump a space
					char *key = get_line(stdin);
					record = find(root, key);
					free(key);
					if (!record) {
						printf("Not found\n");
					}else{
						print_record(record);
					}
					printf("Completed within %.6fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
					ungetc('\n', stdin);
				}
				break;
			}
			case 'p':
				print_tree(root);
				break;
			case '-':{  // parse command and jump to stdin
				int i = 0;
				char buff[1024];
				int argc;
				char **argv;
				ungetc('-', stdin);
				while ('\n'!=(command=fgetc(stdin))){
					buff[i] = command;
					i++;
					if (i == 1024) break;
				}
				buff[i] = '\0';
				argc = std_arg_parser(buff, &argv);
				root = stdin_mode(argc, argv);
				free(argv);
				ungetc('\n', stdin);
				break;
			}
			case '\n':
				printf("> ");
				break;
			default:
				printf("unknown command:%c\n", command);
				break;
		}
	// while (fgetc(stdin)!='\n')
	// 	;
	

	}
}

int main(int argc, char const *argv[])
{
	Node *root = NULL;
	print_welcome();
	if (argc > 1)
	{
		root = stdin_mode(argc, argv);
	}
	interactive_mode(root);
	return 0;
}

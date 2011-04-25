/************************
 * util.c
 *
 * utility functions
 *
 ************************/

#include "util.h"

/***************
 * These functions are just some handy file functions.
 * We have not yet covered opening and reading from files in C,
 * so we're saving you the pain of dealing with it, for now.
 *******/
FILE* file_open(char* filename) {
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, "Error opening file %s, abort.\n", filename);
		exit(1);
	}
	return fp;
}

/*******************
 * To use this function properly, create a char* and malloc 
 * 160 bytes for it. Then pass that char* in as the argument
 ******************/
char* file_getline(char* buffer, FILE* fp) {
	buffer = fgets(buffer, 160, fp);
	return buffer;
}

/************
 * str_node creation, creates a copy of given string
 ****************/
str_node_t* str_node_create(char* str) {
	str_node_t* rnode = malloc(sizeof(str_node_t));
	rnode->str = strdup(str);
	rnode->next = NULL;
	return rnode;
}

/*************
 * Recursive function for freeing a str_node
 *************/
void str_node_free(str_node_t* node) {
	if (node->str != NULL) { free(node->str); }
	if(node->next != NULL) { str_node_free(node->next); }
	free(node);
}

/*************
 * Create a rule_t
 *************/
rule_t* rule_create(char* target) {
	rule_t* rule = malloc(sizeof(rule_t));
	rule->target = strdup(target);
	rule->deps = NULL;
	rule->commandlines = NULL;
	return rule;
}

/************
 * add a dependency to a rule
 *************/
void rule_add_dep(rule_t* rule, char* dep) {
	if(rule->deps == NULL) { rule->deps = str_node_create(dep); }
	else {
		str_node_t* node;
		for(node = rule->deps; node->next != NULL; node = node->next);
		node->next = str_node_create(dep);
	}
}

/*************
 * add a command line to a rule
 ***************/
void rule_add_commandline(rule_t* rule, char* cmd) {
	if(rule->commandlines == NULL) { rule->commandlines = str_node_create(cmd); }
	else {
		str_node_t* node;
		for(node = rule->commandlines; node->next != NULL; node = node->next);
		node->next = str_node_create(cmd);
	}
}

/**********
 * free a rule
 **********/
void rule_free(rule_t* rule) {
	if(rule->deps != NULL) { str_node_free(rule->deps); }
	str_node_free(rule->commandlines);
	free(rule->target);
	free(rule);
}

/**************
 * create a rule node
 ***************/
rule_node_t* rule_node_create(rule_t* rule) {
	rule_node_t* rnode = malloc(sizeof(rule_node_t));
	rnode->next = NULL;
	rnode->rule = rule;
	return rnode;
}

/**************
 * free a rule_node
 *************/
void rule_node_free(rule_node_t* node) {
	rule_free(node->rule);
	if(node->next != NULL) { rule_node_free(node->next); }
	free(node);
}

/*********************
 * trim white space from front and back of a string
 **********************/
char* trim(char* str) {
	int start, end, i;
	for(start = 0; isspace(str[start]); start++);
	for(end = strlen(str)-1; isspace(str[end]); end--);
	char* retstr = calloc(sizeof(char), end - start + 2);
	for(i = start; i <= end; i++) { retstr[i - start] = str[i]; }
	retstr[end - start + 1] = '\0';
	return retstr;
}


/******************
 * create an argument vector given a command line
 *****************/
char** get_argvec(char* cmd) {
	char* working = trim(cmd);
	char* ptr = working;
	int count = 0;
	while(*ptr != '\0') {
		for(;*ptr != '\0' && isspace(*ptr); ptr++);
		for(;*ptr != '\0' && !isspace(*ptr); ptr++);
		count++;
	}
	//count--;
	char** rvec = calloc(count+1, sizeof(char*));
	ptr = working; count = 0;
	while(*ptr != '\0') {
		for(;*ptr != '\0' && isspace(*ptr); ptr++);
		char* start = ptr;
		for(;*ptr != '\0' && !isspace(*ptr); ptr++);
		char* end = ptr;
		rvec[count] = calloc((end - start + 1), sizeof(char));
		strncpy(rvec[count], start, (end - start));
		count++;
	}
	rvec[count] = NULL;
	free(working);
	return rvec;
}

/************
 * create a pid list
 ***********/
pid_node_t* pid_node_create(pid_t pid) {
	pid_node_t* rnode = malloc(sizeof(pid_node_t));
	rnode->next = NULL;
	rnode->pid = pid;
	return rnode;
}

/**************
 * free a pid list
 **************/
void pid_node_free(pid_node_t* node) {
	if(node->next != NULL) { pid_node_free(node->next); }
	free(node);
}

void error(char* message)
{
    printf("%s\n", message);
    exit(1);
}

size_t send_message(int fd, int id, char* message)
{
    uint32_t header = htonl(id);
    size_t message_len = strlen(message) + 1;
    uint32_t len = htonl(message_len);
    
    size_t bytes_written = 0;
    //printf("Sending (%d %d), (%d %d), (%s)\n", id, header, message_len, len, message);
    bytes_written += write(fd, (void*)&header, sizeof(uint32_t));
    bytes_written += write(fd, (void*)&len, sizeof(uint32_t));
    bytes_written += write(fd, (void*)message, message_len);
    
    return bytes_written;
}

DIME_MESSAGE* receive_message(int sockfd)
{
    char id_buffer[4];
	ssize_t bytes_read = read(sockfd, (void *) &id_buffer, 4);
	if (bytes_read == 0) //EOF
	{
	    return NULL;
	}
	else if (bytes_read < 4)
	{
	    printf("Read %d bytes\n", bytes_read);
		error("Error reading message ID");
    }
	uint32_t id = ntohl(*((uint32_t *)id_buffer));
		
	char payload_buffer[4];
	bytes_read = read(sockfd, (void *) &payload_buffer, 4);
	if (bytes_read < 4)
	{
		error("Error reading message length");
	}
	uint32_t payload = ntohl(*((uint32_t *)payload_buffer));
	
	char* message_buffer = (char*)malloc(sizeof(char) * payload);
	//printf("Allocated %p\n", message_buffer);
    bytes_read = read(sockfd, (void *)message_buffer, payload);
    
    DIME_MESSAGE* message = (DIME_MESSAGE*)malloc(sizeof(DIME_MESSAGE));
    message->id = id;
    message->len = payload;
    message->message = message_buffer;
    
    //printf("Received %d, %d, %s\n", id, payload, message->message);
    
    return message;
}

void message_free(DIME_MESSAGE* message)
{
	//printf("Freeing %p\n", message->message);
    free(message->message);
    free(message);
}

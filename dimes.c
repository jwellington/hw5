/*******************************
 * dimes.c
 *
 * Source code for DIstributed MakE Server
 *
 ******************************/

#include "util.h"
#include "dimes.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/*********
 * Simple usage instructions
 *********/
void dime_usage(char* progname) {
	fprintf(stderr, "Usage: %s [options] [target]\n", progname);
	fprintf(stderr, "-f FILE\t\tRead FILE as a dimefile.\n");
	fprintf(stderr, "-h\t\tPrint this message and exit.\n");
	fprintf(stderr, "-n\t\tDon't actually execute commands, just print them.\n");
	exit(0);
}

/****************************** 
 * this is the function that, when given a proper filename, will
 * parse the dimefile and read in the rules
 ***************/
rule_node_t* parse_file(char* filename) {
	char* buffer = malloc(160*sizeof(char));
	char* line;
	FILE* fp = file_open(filename);
	rule_node_t* rlist = NULL;
	rule_t* current_rule = NULL;
	while((line = file_getline(buffer, fp)) != NULL) {
		if(line[0] == '#') {
			// do nothing because this line is a comment
		} else {
			// not a comment
			if(current_rule != NULL) {
				if(line[0] == '}') {
					// then this is the closing } of a rule
					rule_node_t* node = rule_node_create(current_rule);
					node->next = rlist;
					rlist = node;
					current_rule = NULL;
				} else {
					// this is just another command line in the rule
					char* trim_line = trim(line);
					rule_add_commandline(current_rule, trim_line);
					free(trim_line);
				}
			} else {
				if(strchr(line, ':') != NULL) {
					// this is the start of a rule
					char* trim_targ = trim(strtok(line, ":"));
					current_rule = rule_create(trim_targ);
					free(trim_targ);
					char* alldeps = strtok(NULL, ":");
					char* dep = strtok(alldeps, " ");
					if(dep != NULL) {
						do {
							if(*dep != '{') {
								rule_add_dep(current_rule, dep);
							}
						} while((dep = strtok(NULL, " ")) != NULL);
					}
				}
			}
		}
	}
	fclose(fp);
	free(buffer);
	return rlist;
}

/**************
 * Exec targets with recursive calls
 **************/
void exec_target_rec(rule_t* rule, rule_node_t* list, int sockfd) {
	str_node_t* sptr;
	rule_node_t* rptr;
	for(sptr = rule->deps; sptr != NULL; sptr = sptr->next) {
		// for each dependency, see if there's a rule, then exec that rule
		for(rptr = list; rptr != NULL; rptr = rptr->next) {
			if(strcmp(sptr->str, rptr->rule->target) == 0) {
				exec_target_rec(rptr->rule, list, sockfd);
			}
		}
	}

	fake_exec(rule, sockfd);
}

/***********
 * Function for 'fake execing' for HW4.
 * Don't exec, just print commandlines and wait 1 sec per commandline
 ***********/
void fake_exec(rule_t* rule, int sockfd) {
	str_node_t* sptr;
	for(sptr = rule->commandlines; sptr != NULL; sptr = sptr->next) {
		send_message(sockfd, 103, sptr->str);
		//printf("%s\n", sptr->str);
		usleep(500000);
	}
}

/*********
 * Given a target list and the list of rules, execute the targets.
 *********/
void execute_targets(int targetc, char* targetv[], rule_node_t* list, int sockfd) {
	rule_node_t* ptr = list;
	int i;
	if(targetc == 0) {
		// no target specified on command line!
		for(ptr = list; ptr->next != NULL; ptr = ptr->next);
		if(ptr == NULL) {
			fprintf(stderr, "Error, no targets in dimefile.\n");
		} else {
			exec_target_rec(ptr->rule, list, sockfd);
		}
	} else {
		for(i = 0; i < targetc; i++) {
			for(ptr = list; ptr != NULL; ptr = ptr->next) {
				if(strcmp(targetv[i], ptr->rule->target) == 0) {
					exec_target_rec(ptr->rule, list, sockfd);
					break;
				}
			}
			if(ptr == NULL) {
			    char errbuf[strlen("Error, target '' not found.\n") + strlen(targetv[i]) + 1];
			    sprintf(errbuf, "Error, target '%s' not found.\n",targetv[i]);
				send_message(sockfd, 105, errbuf);
				printf("%s", errbuf);
			}
		}
	}
}

int main(int argc, char* argv[]) {
	// Declarations for getopt
	extern int optind;
	extern char* optarg;
	int sock, length;
	//socklen_t clilen;
	unsigned int clilen;
	struct sockaddr_in server;
	struct sockaddr_in client;
	
	// Variables you'll want to use
	char* filename = "Dimefile";
	rule_node_t* list = parse_file(filename);
	
	if(argc != 2)
	{
		error("Usage error");
	}

	// parse the given file, then execute targets
	sock=socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) 
		error("Opening socket");
	length = sizeof(server);
	bzero(&server,length);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(atoi(argv[1]));
	if (bind(sock,(struct sockaddr *)&server,length)<0) 
	    error("Failed to bind socket");
	clilen = sizeof(struct sockaddr_in);
	
	listen(sock, 100);
	
	while(1)
	{
		int new_socket = accept(sock, (struct sockaddr *) &client, &clilen);
		int port_num = ntohs(client.sin_port);
		char* client_ip = inet_ntoa(client.sin_addr);
		printf("Incoming connection from %s at port %d\n", client_ip, port_num);
		
	    while (1)
	    {        
            DIME_MESSAGE* message = receive_message(new_socket);
            if (message == NULL) //Client disconnected
            {
                break;
            }
            uint32_t id = message->id;
            uint32_t payload = message->len;
            char message_buffer[strlen(message->message) + 1];
            strcpy(message_buffer, message->message);
            message_free(message);
		
		    //printf("Header is: %d --- %d\n", id,  payload);
		    if(id == 100) //Handshake received
		    {
			    //printf("100\n");
			    //printf("Handshake received: %s\n", message_buffer);
			    send_message(new_socket, 101, "");
			    //Handshake Response
		    }
		    else if(id == 102)
		    {
			    printf("Received request to execute: %s from %s\n", message_buffer, client_ip);
		        //FIXME: Actually execute targets here
		        if (strlen(message_buffer) > 0)
		        {
		            char* tar_list[0];
		            tar_list[0] = message_buffer;
		            execute_targets(1, tar_list, list, new_socket);
		        }
		        else
		        {
		            execute_targets(0, NULL, list, new_socket);
		        }
			    send_message(new_socket, 104, "");
			    //Execute Target
		    }
		    else if(id == 105)
		    {
			    printf("%s\n", message_buffer);
		    }
		    else
		    {
		        printf("Unrecognized message ID: %d\n", id);
		    }
		}
		printf("%s disconnected.\n", client_ip);
		close(new_socket);
	}
	
	//execute_targets(argc, argv, list);
	rule_node_free(list);
	return 0;
}


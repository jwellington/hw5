/**************************
 * dimes.h -- the header file for dimes.c, a 
 * distributed make server
 *
 *
 *
 *************************/

#ifndef _DIMES_H_
#define _DIMES_H_

#define true 1
#define false 0
typedef int bool;

void dime_usage(char*);
rule_node_t* parse_file(char*);
void fake_exec(rule_t*, int);
void exec_target_rec(rule_t*, rule_node_t*, int);
void execute_targets(int, char**, rule_node_t*, int);
void* process_client(void*);
void handle_sigpipe(int);

#endif

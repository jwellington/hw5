/*******************************
 * dimec.c
 *
 * Source code for DIstributed MakE Client
 *
 ******************************/

#include "dimec.h"
#include "util.h"
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

int main(int argc, char* argv[]) {

    char* usage = "./dimec SERVER_ADDR PORT_NUM [TARGET1] [TARGET 2] ...";

    if (argc < 3)
    {
        printf("Usage:\n%s", usage);
        error("Not enough arguments");
    }
    char* server_addr = argv[1];
    
    int port_num = atoi(argv[2]);
    
    str_node_t* targets = NULL;
    if (argc > 3)
    {
        str_node_t* cur_target = (str_node_t*)malloc(sizeof(str_node_t));
        cur_target->next = NULL;
        cur_target->str = argv[3];
        targets = cur_target;
        int i;
        for (i = 4; i < argc; i++)
        {
            cur_target->next = (str_node_t*)malloc(sizeof(str_node_t));
            cur_target = cur_target->next;
            char* tar_str = (char*)malloc(strlen(argv[i]) + 1);
            strcpy(tar_str, argv[i]);
            cur_target->str = tar_str;
            cur_target->next = NULL;
        }
    }
    else
    {
        targets = (str_node_t*)malloc(sizeof(str_node_t));
        targets->next = NULL;
        char* tar_str =(char *)malloc(1);
        strcpy(tar_str, "");
        targets->str = tar_str;
    }
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0)
	{
	    error("Failed to set up socket");
	}
	
	struct sockaddr_in my_addr;
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = INADDR_ANY;
	my_addr.sin_port = htons(4118);
	
	int err = bind(sockfd, (const struct sockaddr *)&my_addr, sizeof(struct sockaddr_in));
	
	if (err < 0)
	{
	    error("Failed to bind socket");
	}
	
	struct sockaddr_in serv_addr;
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port_num);
	
    struct hostent* hp = gethostbyname(server_addr);
    if (hp == 0)
    {
        error("Unknown host");
    }
    bcopy((char*)hp->h_addr,
          (char*)&(serv_addr.sin_addr),
          hp->h_length);
	
	err = connect(sockfd, (const struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in));
	if (err < 0)
	{
	    error("Failed to connect");
	}
	else
    {
        printf("Connected to %s at port %d.\n", server_addr, port_num);
    }
	size_t bytes_sent = send_message(sockfd, 100, "");
	
	int done = 0;
	str_node_t* cur_target = targets;
	
	while (!done)
	{
		/*char id_buffer[4];
		ssize_t bytes_read = read(sockfd, (void *) &id_buffer, 4);
		if(bytes_read < 4)
			error("Error reading message ID");
		uint32_t id = ntohl(*((uint32_t *)id_buffer));
			
		char payload_buffer[4];
		bytes_read = read(sockfd, (void *) &payload_buffer, 4);
		if(bytes_read < 4)
			error("Error reading message length");
		uint32_t payload = ntohl(*((uint32_t *)payload_buffer));
		
		char message_buffer[payload];
        bytes_read = read(sockfd, (void *) &message_buffer, payload);*/
        
        DIME_MESSAGE* message = receive_message(sockfd);
        uint32_t id = message->id;
        uint32_t payload = message->len;
        char* message_buffer = message->message;
        message_free(message);
        
		printf("Header is: %d --- %d\n", id,  payload);
        if (id == 101) //Handshake response
        {
            char* target_str = cur_target->str;
            printf("Sending target %s\n", target_str);
            send_message(sockfd, 102, target_str);
            cur_target = cur_target->next;
        }
        else if (id == 103)
        {
            while (id != 104)
            {
                char* response = message_buffer;
                printf("Received response %s\n", response);
                message = receive_message(sockfd);
                id = message->id;
                message_free(message);
            }
            if (cur_target == NULL)
            {
                printf("Done\n");
                done = 1;
            }
            else
            {
                char* target_str = cur_target->str;
                printf("Sending target %s\n", target_str);
                send_message(sockfd, 102, target_str);
                cur_target = cur_target->next;
            }
        }
	    else if(id == 105)
	    {
		    error(message_buffer);
	    }
	    else
	    {
	        printf("Unrecognized message ID %d\n", id);
	    }
	}
	
	str_node_free(targets);
	close(sockfd);
	
	return 0;
}

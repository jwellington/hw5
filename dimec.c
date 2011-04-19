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
    //FIXME: Get targets
	
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
	uint32_t header = htonl(100);
	uint32_t len = htonl(7);
	write(sockfd, (void*)&header, sizeof(uint32_t));
	write(sockfd, (void*)&len, sizeof(uint32_t));
	char* message = "Message";
	write(sockfd, (void*)message, len);
	
	close(sockfd);
	
	return 0;
}

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define BUFLEN 1024

void error(const char *);
int main(int argc, char *argv[])
{
   int sock, n;
   unsigned int length;
   struct sockaddr_in server, from;
   struct hostent *h;
   char buffer[BUFLEN];
   
   if (argc != 3) { printf("Usage: server port\n");
                    exit(1);
   }
   sock= socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) error("socket");

   server.sin_family = AF_INET;
   h = gethostbyname(argv[1]);
   if (h==0) error("Unknown host");

   bcopy((char *)h->h_addr, 
        (char *)&server.sin_addr,
         h->h_length);
   server.sin_port = htons(atoi(argv[2]));
   length=sizeof(struct sockaddr_in);
   while (1){
	  /* Get the message from the user
	   * If it is not "quit" then send it to server, else close socket
	   * Receive message from the server
	   * We need to use "sendto" and "recvfrom" from UDP.
	   */ 
   }
   close(sock);
   return 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

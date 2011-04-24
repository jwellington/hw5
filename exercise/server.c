/* Creates a datagram server.  The port 
   number is passed as an argument.  This
   server runs forever */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

#define BUFLEN 1024

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
   int i;
   int sock, length, n;
   //socklen_t clilen;
   unsigned int clilen;
   struct sockaddr_in server;
   struct sockaddr_in client;
   char buf[BUFLEN];

   if (argc < 2) {
      fprintf(stderr, "ERROR, no port provided\n");
      exit(0);
   }
   
   /* Create socket
    * Bind
    */ 
   
   while (1) {
      /* Recieve message from the client.
       * Convert all charcters to upper case
       * Send it back.
       * We need to use "recvfrom" and "sendto" for UDP. 
       */
   }
   return 0;
 }


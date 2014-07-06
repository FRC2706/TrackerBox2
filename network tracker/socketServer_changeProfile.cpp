#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>      // Needed for memset
#include <unistd.h>

#include "networkTrackerYAML_utils.h"

#define MY_IP "127.0.0.1" // find this out programatically?
#define SOCK_CHANGE_PROFILE_PORT 1181 // read this from a config file, or hard-code it?

using namespace std;

int main( int argc, char *argv[] )
{
	printf("Welcome to TrackerBox2's Network Utils.\nThis program listens on port %d"
			" for requests to change vision profile.\n", SOCK_CHANGE_PROFILE_PORT);
			
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int  n;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = SOCK_CHANGE_PROFILE_PORT;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
 
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
                          sizeof(serv_addr)) < 0)
    {
         perror("ERROR on binding");
         exit(1);
    }

    /* Now start listening for the clients, here process will
    * go in sleep mode and will wait for the incoming connection
    */
    listen(sockfd,5);
    clilen = sizeof(cli_addr);

	while(1) {
		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, 
		                            (socklen_t *)&clilen);
		if (newsockfd < 0) 
		{
		    perror("ERROR on accept");
		    exit(1);
		}
		/* If connection is established then start communicating */
		memset(buffer, 0, 256);
		n = recv( newsockfd,buffer,255,0 );
		if (n < 0)
		{
		    perror("ERROR reading from socket");
		    exit(1);
		}
		
		
		printf("Received %d bytes\n", n);

		/* change the active profile in "parameters.yaml" */
		int newProfile = buffer[2] - 48;
		
		if (newProfile >=0 && newProfile <10) {
			printf("Received a request to switch to Profile%d\n",newProfile);
			Parameters p = loadParametersFromFile();
			p.activeProfileIdx = newProfile;
			writeParametersToFile(p);
		} else {
			printf("Received an invalid request\n");
		}	
		close(newsockfd);
	}
    return 0; 
}

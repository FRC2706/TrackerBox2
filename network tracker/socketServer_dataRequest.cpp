#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>      // Needed for memset
#include <unistd.h>
#include <sys/time.h>

#include "networkTrackerYAML_utils.h"

#define MY_IP "127.0.0.1" // find this out programatically?
#define SOCK_CHANGE_DATA_REQUEST_PORT 1182 // read this from a config file, or hard-code it?

using namespace std;

int main( int argc, char *argv[] )
{
	printf("Welcome to TrackerBox2's Network Utils.\nThis program listens on port %d"
			" for requests to change vision profile.\n", SOCK_CHANGE_DATA_REQUEST_PORT);
			
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
    portno = SOCK_CHANGE_DATA_REQUEST_PORT;
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

	ParticleReport pr;
	
	timeval start, ends;
	gettimeofday(&start, 0);
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

//		printf("Received a request for the particle data.\n");
		
		/* read the current particle report from "particleReport.yaml" and send it back */
		gettimeofday(&ends, 0);
		double elapsed = ( (double) (ends.tv_sec - start.tv_sec) + (double) (ends.tv_usec - start.tv_usec) / 1000000);
		start = ends;
		if (elapsed > 0.05) // if we try to read the YAML file while it's still being written from the last time, things get ugly.
			pr = loadParticleReportFromFile();
		
		char msg[256];
		
		n = sprintf(msg, "%f,%f,%f,%f,%f", pr.centerX, pr.centerY, pr.area, pr.velX, pr.velY);
		
		printf("message to send: %s\n", msg);
		
		n = write(newsockfd,msg,n);
		if (n < 0)
		{
		    perror("ERROR writing to socket");
		    exit(1);
		}
		close(newsockfd);	
	}
    return 0; 
}

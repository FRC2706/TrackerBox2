#include "network_utils.h"

#include "networkTracker.h"
#include "YAML_utils.h"

#include <cstdlib>
#include <cstring>      // Needed for memset
#include <netinet/in.h>
#include <pthread.h>	// light multi-threading library
#include <stdio.h>
#include <sys/socket.h> // Needed for the socket functions


/*************** NETWORK SERVER STUFF **********************/

void *runChangeProfileServer(void *placeHolder) {
	printf("Listening on port %d"
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
//			Parameters p = loadParametersFromFile();
			pthread_mutex_lock( &paramsMutex );
			p.activeProfileIdx = newProfile;
			writeParametersToFile(p);
			pthread_mutex_unlock( &paramsMutex );
		} else {
			printf("Received an invalid request\n");
		}
		close(newsockfd);
	}

	pthread_exit(0);
}

void *runDataRequestServer(void *placeHolder) {
	printf("Listening on port %d"
			" for requests to transmit data.\n", SOCK_CHANGE_DATA_REQUEST_PORT);

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

//	ParticleReport pr;

	while(1) {
		printf("Waiting for new connection.\n");
		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
				                        (socklen_t *)&clilen);

		if (newsockfd < 0)
		{
			perror("ERROR on accept");
	//		    exit(1);
			continue;
		}
		printf("Got a new connection!\n");

		struct timeval tv;
		tv.tv_sec = 10; // 10 second timeout
		tv.tv_usec = 0; // you need to initialize this

		setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

	//	timeval start, ends;
	//	gettimeofday(&start, 0);
		while(1) {
		    printf("Waiting for request.\n");
			/* Accept actual connection from the client */
	//		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
	//		                            (socklen_t *)&clilen);
	//		if (newsockfd < 0)
	//		{
	//		    perror("ERROR on accept");
	////		    exit(1);
	//			continue;
	//		}
	//		printf("Got a new connection!");
			/* If connection is established then start communicating */

			memset(buffer, 0, 256);
			n = recv( newsockfd,buffer,255,0 );
			if (n <= 0)
			{
				perror("ERROR reading from socket");
	//		    exit(1);
				break;
			}

			printf("Received a request for the particle data.\n");

			/* read the current particle report from "particleReport.yaml" and send it back */
	//		gettimeofday(&ends, 0);
	//		double elapsed = ( (double) (ends.tv_sec - start.tv_sec) + (double) (ends.tv_usec - start.tv_usec) / 1000000);
	//		start = ends;
	//		if (elapsed > 0.05) // if we try to read the YAML file while it's still being written from the last time, things get ugly.
	//			pr = loadParticleReportFromFile();

			char msg[256];

			pthread_mutex_lock( &mostRecentPRMutex );
			n = sprintf(msg, "%f,%f,%f,%f,%f", mostRecentPR.centerX, mostRecentPR.centerY, mostRecentPR.area, mostRecentPR.velX, mostRecentPR.velY);
			pthread_mutex_unlock( &mostRecentPRMutex );

			printf("message to send: %s\n", msg);

			n = write(newsockfd,msg,n);
			if (n < 0)
			{
				perror("ERROR writing to socket");
				//exit(1);
				break;
			}
			printf("Sent!\n");
		} // inner loop
		close(newsockfd);
		printf("Succesfully closed connection.\n");
	} // outer loop
	pthread_exit(0);
}

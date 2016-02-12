#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

using namespace std;


#ifndef VISION_DATA__H
#define VISION_DATA__H

struct VisionTarget {

  // the area of the bounding box of this target, as a fraction of the total area of the image.
  float boundingArea;     // % of cam [0, 1.0]

  //center of target
  float ctrX;             // [-1.0, 1.0]
  float ctrY;             // [-1.0, 1.0]

  // the aspect ratio of the target we found. This can be used directly as a poor-man's measure of skew.
  float aspectRatio;
};

struct VisionReport {

  // The number of targets that were detected.
  int numTargetsFound;

  // An array of vision targets.
  VisionTarget* targetsFound;
};

#endif


#define VISION_DATA_PORT = 1182;
#define PRINT_STUFF 1

// When ever you call this, remember to free the memory after!
//
// VisionReport* visionReport = getTrackerboxData(rpi_addr);
// ...
// delete[] vr->targetsFound;
// delete vr;
VisionReport* getTrackerboxData(const char* rpi_addr) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;

   char buffer[2048];


   portno = 1182;

   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   server = gethostbyname(rpi_addr);

   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }

   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);

   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }


   /* Send an empty message to the RPi */
   	sprintf(buffer, " ");

   n = write(sockfd, buffer, strlen(buffer));

   if (n < 0) {
      perror("ERROR writing to socket");
      exit(1);
   }

   /* Now read server response */
   bzero(buffer,2048);
   n = read(sockfd, buffer, 2048);

   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }

	#if PRINT_STUFF
   		printf("I got back: \"%s\"\n",buffer);
	#endif

	// parse the string we got back

	// first, count the number of colons so we know how many targets were found
	int numTargetsFound = 0;
	for (int i = 0; i < 2048; i++) {
		if (buffer[i] == ':') numTargetsFound++;
		else if (buffer[i] == '\0') break;
	}

	#if PRINT_STUFF
		printf("We found: %d targets\n", numTargetsFound);
	#endif

	VisionReport* vr = new VisionReport;
	vr->numTargetsFound = numTargetsFound;
	vr->targetsFound = new VisionTarget[numTargetsFound];

	for (int i = 0; i < numTargetsFound; i++) {
		sscanf(buffer, "%f,%f,%f,%f:%s", &(vr->targetsFound[i].ctrX),
										 &(vr->targetsFound[i].ctrY),
										 &(vr->targetsFound[i].aspectRatio),
										 &(vr->targetsFound[i].boundingArea),
									     buffer);
	}

	#if PRINT_STUFF
		if (numTargetsFound >= 1) {
			printf("First target: %.3f,%.3f,%.3f,%.3f\n", vr->targetsFound[0].ctrX,
											 			vr->targetsFound[0].ctrY,
											 			vr->targetsFound[0].aspectRatio,
											 			vr->targetsFound[0].boundingArea);
		}
	#endif

}


int main(int argc, char *argv[]) {

	getTrackerboxData("127.0.0.1");

	return 0;
}

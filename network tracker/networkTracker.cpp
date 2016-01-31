/**
 * Written for the FIRST Robotics Competition
 * Copyright 2014 Mike Ounsworth
 * ounsworth@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/** I am aware that this code is an abhorant mix of c and c++ opencv calls and data structures. **/

#include "networkTracker.h"

#include "networkTrackerYAML_utils.h"
#include "vision_utils.h"

#include <highgui.h>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h> // Needed for the socket functions
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>	// light multi-threading library

#define SHOW_GUI 1
#define PRINT_FPS 1
#define RUN_WGET 1


using namespace cv;
using namespace std;

Parameters p;
pthread_mutex_t paramsMutex = PTHREAD_MUTEX_INITIALIZER;

VisionReport mostRecentVR;
pthread_mutex_t mostRecentPRMutex = PTHREAD_MUTEX_INITIALIZER;

/** Make sure you call
  *	pthread_mutex_lock( &paramsMutex );
  * before you call me!
  */
void loadParams() {
	Parameters newP = loadParametersFromFile();


	pthread_mutex_lock( &paramsMutex );
	if (newP != p) {
		p = newP;
		pthread_mutex_unlock( &paramsMutex );

		writeParametersToFile(newP);
	} else {
		pthread_mutex_unlock( &paramsMutex );
	}
}


int main( int argc, char** argv )
{

	// Start the network server in a seperate thread
	pthread_t threadDataRequest;
	int rc;

	rc = pthread_create(&threadDataRequest, NULL, runDataRequestServer, NULL);
	if (rc){
         cerr << "Error:unable to create thread for the Data Request Server,"<< endl;
	}

	loadParams();

	IplImage* frame;

	 // Note: for examples of how to connect openCV directly to a camera, see older versions of this file on github

	 // spawn a side process to do a web-get to fetch the latest frame of the jpg.
	#if RUN_WGET
		int pid = fork();
		if ( pid == 0 ) {	// in the child process
			// http://i.imgur.com/5aEOlcW.jpg
			printf("Connecing to Axis Cam at %s...", p.ipCamAddr.c_str());
			cout.flush();
			execlp("/usr/bin/wget", "/usr/bin/wget", p.ipCamAddr.c_str(), "-O", WGET_PIC_LOC, "-q", NULL);

			printf("Done!\n\n\n");
		}
	#else
		printf("wget disabled by #define RUN_WGET 0 in networkTracker.cpp\n");
	#endif

	#if PRINT_FPS
		timeval start, ends;
		gettimeofday(&start, 0);
	#endif

	// Main frame loop
	while(1) {

		// load the latest frame from the ramdisk
		frame = cvLoadImage( "/dev/shm/camera.jpg" );

		// make sure that the load did not fail
		if(frame == NULL) {
			//printf("No Image from camera %s\n", p.ipParams.axisCamAddr.c_str());
			continue;
		}

		#if RUN_WGET
			// spawn a side process to do a web-get to fetch the latest frame of the jpg.
			int childpid = fork();
			if ( childpid == 0 ) {
				execlp("/usr/bin/wget", "/usr/bin/wget", p.ipCamAddr.c_str(), "-O", WGET_PIC_LOC, "-q", NULL);
			}
		#endif

		loadParams();

		#if PRINT_FPS
			gettimeofday(&ends, 0);
			cout << "FPS: " << 1.0 / ( (double) (ends.tv_sec - start.tv_sec) + (double) (ends.tv_usec - start.tv_usec) / 1000000) << endl;
			start = ends;
		#endif



		// Do the image processing

		IplImage* mask = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
		thresholdHSV(frame, mask, p.minHue, p.maxHue, p.minSat, p.maxSat, p.minVal, p.maxVal);

		Mat element = cv::getStructuringElement( cv::MORPH_RECT,
		                                       Size( 2*p.erodeDilateSize + 1, 2*p.erodeDilateSize+1 ),
		                                       Point( p.erodeDilateSize, p.erodeDilateSize ) );

		// Apply the erosion-dilation smoothing operations
		cv::Mat matMask(mask);
    cv::erode( matMask, matMask, element );
		cv::dilate( matMask, matMask, element );


		IplImage* outputImage = cvCloneImage(frame);
		VisionReport vr = findFRCVisionTargets(mask, outputImage, p.minTargetArea);

		std::cout << " | " << vr.targetsFound[0].aspectRatio << " | " << vr.targetsFound[0].ctrX << " | " << vr.targetsFound[0].ctrY << " | " << vr.targetsFound[0].boundingArea << " | ";


		#if SHOW_GUI
			cvShowImage("Binary Mask", mask);
			cvShowImage("Result", outputImage);
			cvWaitKey(5);	// give a pause for the openCV GUI to draw
		#endif

		// release the memory of the images we created.
		cvReleaseImage(&frame);
		cvReleaseImage(&mask);
		cvReleaseImage(&outputImage);

		#if RUN_WGET
			int returnStatus;
			waitpid(childpid, &returnStatus, 0);	// -1 means that the parent process will wait for _all_ child processes to terminate. We're only starting 1 child.
		#endif
	} // end video frame loop
} // end main()


/**
 * Takes a BGR image.
 * Returns a binary mask which is the result of performing this threshold.
 */
void thresholdHSV(IplImage* image, IplImage* mask, unsigned char minH, unsigned char maxH, unsigned char minS, unsigned char maxS, unsigned char minV, unsigned char maxV) {
	// convert image to HSV space
	IplImage* hsv = cvCreateImage(cvGetSize(image), image->depth, 3);
	cvCvtColor(image, hsv, CV_BGR2HSV);

	cvInRangeS(hsv, cvScalar(minH, minS, minV), cvScalar(maxH, maxS, maxV), mask);

  cvReleaseImage(&hsv);
}


/*************** NETWORK SERVER STUFF **********************/

/**
 * Accpet an empty message from the roboRIO and return the vision data from the most recent
 * frame that was processed.
 */
void *runDataRequestServer(void *placeHolder) {
	printf("Listening on port %d"
			" for requests to transmit data.\n", SOCK_DATA_REQUEST_PORT);

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
    portno = SOCK_DATA_REQUEST_PORT;
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
				break;
			}

			printf("Received a request for the particle data.\n");

			/* read the current particle report from "particleReport.yaml" and send it back */
			char msg[256];

			pthread_mutex_lock( &mostRecentPRMutex );
			// commented out because I changed what's in the Report struct
		//  n = sprintf(msg, "%f,%f,%f,%f,%f", mostRecentPR.centerX, mostRecentPR.centerY, mostRecentPR.area, mostRecentPR.velX, mostRecentPR.velY);
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

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
#define RUN_WGET 0


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





		// Do some processing on the image

		IplImage* mask = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
		thresholdHSV(frame, mask, p.minHue, p.maxHue, p.minSat, p.maxSat, p.minVal, p.maxVal);

		Mat element = cv::getStructuringElement( cv::MORPH_RECT,
		                                       Size( 2*p.erodeDilateSize + 1, 2*p.erodeDilateSize+1 ),
		                                       Point( p.erodeDilateSize, p.erodeDilateSize ) );

		// Apply the dilation operations
		cv::Mat matMask(mask);
		cv::dilate( matMask, matMask, element );
		cv::erode( matMask, matMask, element );

		IplImage* outputImage = cvCloneImage(frame);
		findFRCVisionTargets(mask, outputImage, p.minTargetArea);


		// compute the center of mass of the target we found
		// TODO I need to make a new version of this for this year's game
//		computeParticleReport(mask);


		// Now maybe draw a dot and arrow for the COM and vel
//		IplImage* maskPlusCOM = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 3);
// 		CvPoint COM_center;
//		cvCvtColor(mask, maskPlusCOM, CV_GRAY2BGR);
//		cvCircle(maskPlusCOM, COM_center, 15, CV_RGB(0,230,40), -1);
		#if SHOW_GUI
			// cvShowImage("Raw Image", frame);
			cvShowImage("Binary Mask", mask);
			cvShowImage("Result", outputImage);
			cvWaitKey(5);	// give a pause for the openCV GUI to draw
		#endif

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
 * This expects a binary mask. It'do weird things if given a 3-channel image.
 */
 // TODO: once we decide ov a vision report, we can work on this and put it back.
// void computeParticleReport(IplImage* mask) {
// 	pthread_mutex_lock( &mostRecentPRMutex );
// 	ParticleReport prevReport = mostRecentPR;
// 	pthread_mutex_unlock( &mostRecentPRMutex );
//
// 	ParticleReport pr;
//
// 	int xAccum, yAccum, areaAccum;
// 	xAccum = yAccum = areaAccum = 0;
//
// 	// compute COM of the mask!
// 	for (int i = 0; i < mask->width; i++)
// 		for (int j = 0; j < mask->height; j++)
// 			// if this pixel is a 1
// 			if (mask->imageData[mask->widthStep * j + i]) {
// 				areaAccum++;
// 				xAccum += i;
// 				yAccum += j;
// 			}
// 	// average
// 	COM_center.x = pr.centerX = ((double) xAccum) / areaAccum;
// 	COM_center.y = pr.centerY = ((double) yAccum) / areaAccum;
//
// 	// This is just here for posterity - turns out that the matrix algebra is actually slower than the loops. *sigh*
// 	//~ COM_center.x = pr.centerX = mean(*Xidxs, Mat(mask)/255.0 ).val[0];
// 	//~ COM_center.y = pr.centerY = mean(*Yidxs, Mat(mask)/255.0 ).val[0];
//
// 	// normalize to [-1, 1]
// 	pr.centerX = (( 2*pr.centerX / mask->width) - 1);
// 	pr.centerY = (( 2*pr.centerY / mask->height) - 1);
// 	pr.area = ((double) cvCountNonZero(mask)) / (mask->width*mask->height);
//
// 	// smooth a little bit
// 	float alpha = 0.4;
// 	pr.velX = alpha*(pr.centerX - prevReport.centerX) + (1-alpha)*prevReport.velX;
// 	pr.velY = alpha*(pr.centerY - prevReport.centerY) + (1-alpha)*prevReport.velY;
//
// 	// check for NANs (ie divide-by-zero) - this is probably completely unnecessary
// 	if(isnan(pr.centerX)) pr.centerX = 0.0;
// 	if(isnan(pr.centerY)) pr.centerY = 0.0;
// 	if(isnan(pr.area)) pr.area = 0.0;
// 	if(isnan(pr.velX)) pr.velX = 0.0;
// 	if(isnan(pr.velY)) pr.velY = 0.0;
//
// 	pthread_mutex_lock( &mostRecentPRMutex );
// 	mostRecentPR = pr;
// 	writeParticleReportToFile(pr);
// 	pthread_mutex_unlock( &mostRecentPRMutex );
// }

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
			// n = sprintf(msg, "%f,%f,%f,%f,%f", mostRecentPR.centerX, mostRecentPR.centerY, mostRecentPR.area, mostRecentPR.velX, mostRecentPR.velY);
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

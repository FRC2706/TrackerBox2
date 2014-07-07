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

#include <highgui.h>
#include <fstream>
#include <cstring>      // Needed for memset
#include <sys/types.h>
#include <sys/socket.h> // Needed for the socket functions
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>	// light multi-threading library
#include "yaml-cpp/yaml.h"

#define SHOW_GUI 1
#define PRINT_FPS 1

using namespace cv;
using namespace std;

#define MY_IP "127.0.0.1" // find this out programatically?
#define SOCK_CHANGE_PROFILE_PORT 1181 // read this from a config file, or hard-code it?
#define SOCK_CHANGE_DATA_REQUEST_PORT 1182 // read this from a config file, or hard-code it?

void *runChangeProfileServer(void *placeHolder);
void *runDataRequestServer(void *placeHolder);

Parameters p;
ProfileParameters activeProfile;
int activeProfileSlider = 0;
pthread_mutex_t paramsMutex = PTHREAD_MUTEX_INITIALIZER;

ParticleReport mostRecentPR;
pthread_mutex_t mostRecentPRMutex = PTHREAD_MUTEX_INITIALIZER;



// This is just here for posterity - turns out that the matrix algebra is actually slower than the loops. *sigh*

//~ /** These matrices are the same size as the image and store the x-coords and y-coords of each point respectively.
 //~ * They are used for computing the centre of mass of the mask **/
//~ Mat* Xidxs;
//~ Mat* Yidxs;
//~ 
//~ void initializeMats(IplImage* frame) {
	//~ Xidxs = new Mat(cvSize(frame->width, frame->height), CV_32FC1);
	//~ Yidxs = new Mat(cvSize(frame->width, frame->height), CV_32FC1);
	//~ 
	//~ // this could be done more efficiently by tiling or repeating vectors using opencv calls, 
	//~ // but I'm not super concerned with runtime of a one-time thing.
	//~ for(int i=0; i < frame->width; i++) {
		//~ for(int j=0; j < frame->height; j++) {			
			//~ Xidxs->at<float>(j,i) = i;
			//~ Yidxs->at<float>(j,i) = j;
		//~ }		
	//~ }
//~ }

void writeParams(int x) { 
	pthread_mutex_lock( &paramsMutex );
	p.profiles[p.activeProfileIdx] = activeProfile;
	writeParametersToFile(p);
	pthread_mutex_unlock( &paramsMutex );
}

void updateTrackbars() {
	#if SHOW_GUI
	cvSetTrackbarPos("minH", "Binary Mask", activeProfile.minH);
	cvSetTrackbarPos("maxH", "Binary Mask", activeProfile.maxH);
	cvSetTrackbarPos("size of noise filter pass", "Binary Mask", activeProfile.noiseFilterSize);
	//cvSetTrackbarPos("size of smoother pass", "Binary Mask", activeProfile.smootherSize);
	cvSetTrackbarPos("Profile #", "Binary Mask", p.activeProfileIdx);
	#else
	return;
	#endif
}

/** Make sure you call 
  *	pthread_mutex_lock( &paramsMutex );
  * before you call me!
  */
void changeProfile(int x) {
	p.profiles[p.activeProfileIdx] = activeProfile;
	p.activeProfileIdx = activeProfileSlider;
	activeProfile = p.profiles[p.activeProfileIdx];
	
	writeParametersToFile(p);
	
	updateTrackbars();
}


/** Make sure you call 
  *	pthread_mutex_lock( &paramsMutex );
  * before you call me!
  */
void loadParams() {
	Parameters newP = loadParametersFromFile();
	if (newP != p) {
		writeParametersToFile(newP);
		p = newP;
	}
	activeProfile = p.profiles[p.activeProfileIdx];
	activeProfileSlider = p.activeProfileIdx;
//	updateTrackbars();
}

CvPoint COM_center;

int main( int argc, char** argv )
{
	// Start the network servers in seperate threads
	pthread_t threadChangeProfile;
	pthread_t threadDataRequest;
	int rc;
	
	rc = pthread_create(&threadChangeProfile, NULL, runChangeProfileServer, NULL);
	if (rc){
         cout << "Error:unable to create thread for the Change Profile Server,"<< endl;
	}
	
	rc = pthread_create(&threadDataRequest, NULL, runDataRequestServer, NULL);
	if (rc){
         cout << "Error:unable to create thread for the Data Request Server,"<< endl;
	}
	
	// now the OpenCV stuff
	#if SHOW_GUI
	cvNamedWindow("Binary Mask", CV_WINDOW_AUTOSIZE);
	cvCreateTrackbar( "minH", "Binary Mask", &activeProfile.minH, 255, writeParams);
	cvCreateTrackbar( "maxH", "Binary Mask", &activeProfile.maxH, 255, writeParams);
	cvCreateTrackbar( "size of noise filter pass", "Binary Mask", &activeProfile.noiseFilterSize, 25, writeParams);
	//cvCreateTrackbar( "size of smoother pass", "Binary Mask", &activeProfile.smootherSize, 25, writeParams);
	cvCreateTrackbar( "Profile #", "Binary Mask", &activeProfileSlider, 9, changeProfile);
	cvWaitKey(5);
	#endif
    
    // copy the parameters.yaml file from the local dir to ramdisk at /dev/shm
	std::ifstream  src("TrackerBox2_parameters.yaml", std::ios::binary);
	std::ofstream  dst("/dev/shm/TrackerBox2_parameters.yaml",   std::ios::binary);
	dst << src.rdbuf();
	src.close();
	dst.close();
    
    pthread_mutex_lock( &paramsMutex );    
	loadParams();
	pthread_mutex_unlock( &paramsMutex );

	IplImage* frame;
	CvCapture* capture;


	// TODO: Select which camera in the GUI. This will need some non-trivial changes to allow it on the fly.
	
	//~ printf("Connecing to Axis Cam at %s...", p.ipParams.axisCamAddr.c_str());
	//~ cout.flush();
	//~ capture = cvCaptureFromFile(p.ipParams.axisCamAddr.c_str());
//	capture = cvCaptureFromFile("vid.avi");
	printf("Opening USB webcam...");
	cout.flush();
	capture = cvCaptureFromCAM(0); // laptop's webcam
	printf("Done!\n\n\n");
	
	frame = cvQueryFrame( capture );
	 
	IplImage* mask;
	IplImage* mask1;
	IplImage* mask2;
		
	initializeMats(frame);

	#if PRINT_FPS
	timeval start, ends;
	gettimeofday(&start, 0);
	#endif
	while(1) {
		frame = cvQueryFrame( capture );
		
		loadParams();
		updateTrackbars();
		
		#if PRINT_FPS
		gettimeofday(&ends, 0);
		//~ cout << "FPS: " << 1.0 / ( (double) (ends.tv_sec - start.tv_sec) + (double) (ends.tv_usec - start.tv_usec) / 1000000) << endl;
		cout << 1.0 / ( (double) (ends.tv_sec - start.tv_sec) + (double) (ends.tv_usec - start.tv_usec) / 1000000) << endl;
		
		start = ends;
		#endif
		
		// Do some processing on the image
		
		mask = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
		
		pthread_mutex_lock( &paramsMutex );
		int minH = activeProfile.minH;
		int maxH = activeProfile.maxH;
		pthread_mutex_unlock( &paramsMutex );
		
		// Threshold the image (if min > max then take the outside region)
		if (minH < maxH)
			thresholdHSV(frame, mask, minH, maxH, 40, 255, 40, 255);
		else {
			mask1 = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
			mask2 = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
			
			thresholdHSV(frame, mask1, 0, maxH, 40, 255, 40, 255);
			thresholdHSV(frame, mask2, minH, 255, 40, 255, 40, 255);
			
			cvOr(mask1, mask2, mask);
			cvReleaseImage(&mask1);
			cvReleaseImage(&mask2);
		}
		
		cvSmooth(mask, mask, CV_MEDIAN, 2*activeProfile.noiseFilterSize+1);
		
//		cvShowImage("Binary Mask", mask);
		
		// compute the center of mass of the target we found
		computeParticleReport(mask);
		
		// Now maybe draw a dot and arrow for the COM and vel
		IplImage* maskPlusCOM = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 3);
		cvCvtColor(mask, maskPlusCOM, CV_GRAY2BGR);
		cvCircle(maskPlusCOM, COM_center, 15, CV_RGB(0,230,40), -1);
		#if SHOW_GUI
			//~ cvShowImage("Raw Image", frame);
			cvShowImage("Binary Mask", maskPlusCOM);
		#endif
		
		// save both the raw image and maskPlusCOM to /tmp so that the web interface can show them
		cvSaveImage("/dev/shm/TrackerBox2_rawImage.jpg", frame);
		cvSaveImage("/dev/shm/TrackerBox2_maskPlusCOM.jpg", maskPlusCOM);
		cvReleaseImage(&maskPlusCOM);
		cvReleaseImage(&mask);	
		cvWaitKey(5);
	} // video frame loop
	
}

/**
 * This expects a binary mask. It'do weird things if given a 3-channel image.
 */
void computeParticleReport(IplImage* mask) {
	pthread_mutex_lock( &mostRecentPRMutex );
	ParticleReport prevReport = mostRecentPR;
	pthread_mutex_unlock( &mostRecentPRMutex );
	
	ParticleReport pr;
	
	int xAccum, yAccum, areaAccum;
	xAccum = yAccum = areaAccum = 0;
	//~ 
	//~ // compute COM of the mask
	//~ // TODO: this can be vectorized!!
	for (int i = 0; i < mask->width; i++)
		for (int j = 0; j < mask->height; j++)
			// if this pixel is a 1
			if (mask->imageData[mask->widthStep * j + i]) {
				areaAccum++;	// cvSum(mask);
				xAccum += i;
				yAccum += j;
			}	
	// average
	COM_center.x = pr.centerX = ((double) xAccum) / areaAccum;
	COM_center.y = pr.centerY = ((double) yAccum) / areaAccum;
	
	// This is just here for posterity - turns out that the matrix algebra is actually slower than the loops. *sigh*
	//~ COM_center.x = pr.centerX = mean(*Xidxs, Mat(mask)/255.0 ).val[0];
	//~ COM_center.y = pr.centerY = mean(*Yidxs, Mat(mask)/255.0 ).val[0];
	
	// normalize to [-1, 1]
	pr.centerX = (( 2*pr.centerX / mask->width) - 1);
	pr.centerY = (( 2*pr.centerY / mask->height) - 1);
	pr.area = ((double) cvCountNonZero(mask)) / (mask->width*mask->height);
	
	// smooth a little bit
	float alpha = 0.4;
	pr.velX = alpha*(pr.centerX - prevReport.centerX) + (1-alpha)*prevReport.velX;
	pr.velY = alpha*(pr.centerY - prevReport.centerY) + (1-alpha)*prevReport.velY;
	
	// check for NANs (ie divide-by-zero) - this is probably completely unnecessary
	if(isnan(pr.centerX)) pr.centerX = 0.0;
	if(isnan(pr.centerY)) pr.centerY = 0.0;
	if(isnan(pr.area)) pr.area = 0.0;
	if(isnan(pr.velX)) pr.velX = 0.0;
	if(isnan(pr.velY)) pr.velY = 0.0;
	
	pthread_mutex_lock( &mostRecentPRMutex );
	mostRecentPR = pr;
	writeParticleReportToFile(pr);
	pthread_mutex_unlock( &mostRecentPRMutex );
}

/**
 * Takes an BGR image.
 *
 * TODO allow special cases of White (high Saturation) and Black (low Value).
 * TODO Search for multiple colours (threshold for each colour and AND their masks together). This will also require modifying the YAML format to have an arbitrary number of colours per profile.
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

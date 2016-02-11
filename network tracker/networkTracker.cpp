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
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>	// light multi-threading library

#define SHOW_GUI 0
#define PRINT_FPS 1
#define PRINT_NETWORK_DEBUGGING 1

// 0 = No Camera, file from disk (camera.jpg)
// 1 = IP Camera (or fetch image from web address)
// 2 = USB Camera (or internal laptop cam)
#define CAMERA_TYPE 1


using namespace cv;
using namespace std;

Parameters p;
pthread_mutex_t paramsMutex = PTHREAD_MUTEX_INITIALIZER;

// Thread-shared variable, needs a mutex
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


	// Set up the camera - based on what type of camera we're using
	#if CAMERA_TYPE == 0
		// No Camera, file from disk (camera.jpg)

		// make sure there's a file called camera.jpg
		if( access( "camera.jpg", F_OK ) ) {
			printf("To run without a camera attached, please place a file called 'camera.jpg' in the current directory.\n");
			exit(-1);
		}

	#elif CAMERA_TYPE == 1
		// IP Camera (or fetch image from web address)

		// spawn a side process to do a web-get to fetch the latest frame of the jpg.
		int childpid = fork();
		if ( childpid == 0 ) {	// in the child process
			// http://i.imgur.com/5aEOlcW.jpg
			printf("Connecing to IP Cam at %s...", p.ipCamAddr.c_str());
			cout.flush();
			execlp( WGET_COMMAND );
			exit(0);	// note that exec only returns if there was an error, on success it calls exit(0) and kills the thread.
						// In the error case we want to exit also
		}

		// in the parent process, wait for the wget process to either succeed, or fail.
		int returnStatus;
		waitpid(childpid, &returnStatus, 0);	// -1 means that the parent process will wait for _all_ child processes to terminate. We're only starting 1 child.
		printf("Done. Camera Connected!\n");

	#elif CAMERA_TYPE == 2
		// USB Camera (or internal laptop cam)

    CvCapture* capture;
		printf("Opening USB webcam...");
		cout.flush();
		capture = cvCaptureFromCAM(0); // laptop's webcam
		frame = cvQueryFrame( capture );
		printf("Done!\n\n\n");
	#else
		printf("Invalid option set for #define CAMERA_TYPE");
		exit -1;
	#endif


	#if PRINT_FPS
		timeval start, ends;
		gettimeofday(&start, 0);
	#endif

	// Main frame loop
	while(1) {

		// Load the new frame - based on what type of camera we're using
		#if CAMERA_TYPE == 0
			// No Camera, file from disk (camera.jpg)
			frame = cvLoadImage( "camera.jpg");

		#elif CAMERA_TYPE == 1
			// IP Camera (or fetch image from web address)

			// load the latest frame that was fetched (load it from the ramdisk)
			frame = cvLoadImage( WGET_PIC_FILE );

			// make sure that the load did not fail
			if(frame == NULL) {
				//printf("No Image from camera %s\n", p.ipParams.axisCamAddr.c_str());
				continue;
			}

			// spawn a side process to do a web-get to fetch the latest frame of the jpg.
			int childpid = fork();
			if ( childpid == 0 ) {
				execlp( WGET_COMMAND );
			}

		#elif CAMERA_TYPE == 2
			// USB Camera (or internal laptop cam)
			frame = cvQueryFrame( capture );

									printf("HERE!\n");
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
		mostRecentVR = findFRCVisionTargets(mask, outputImage, p.minTargetArea);

		//std::cout << " | " << mostRecentVR.targetsFound[0].aspectRatio << " | " << mostRecentVR.targetsFound[0].ctrX << " | " << mostRecentVR.targetsFound[0].ctrY << " | " << mostRecentVR.targetsFound[0].boundingArea << " | ";
		#if PRINT_FPS
			printf("Found %d targets.  ", mostRecentVR.numTargetsFound);
		#endif

		#if SHOW_GUI
			cvShowImage("Binary Mask", mask);
			cvShowImage("Result", outputImage);
			cvWaitKey(5);	// give a pause for the openCV GUI to draw
		#endif

		cvSaveImage(MASK_FILE, mask);
		cvSaveImage(OUTPUT_FILE, outputImage);


		// release the memory of the images we created.
		#if CAMERA_TYPE != 2
			// in the case of a USB webcam, cvQueryFrame(...) does its own memory management
			cvReleaseImage(&frame);
		#endif
		cvReleaseImage(&mask);
		cvReleaseImage(&outputImage);

		#if CAMERA_TYPE == 1
			// IP Camera (or fetch image from web address)
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
	#if PRINT_NETWORK_DEBUGGING
	printf("Listening on port %d"
			" for requests to transmit data.\n", SOCK_DATA_REQUEST_PORT);
	#endif

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

	// disable Nagle's algorithm - should make the send faster.
	int flag = 0;
	if(setsockopt(sockfd,IPPROTO_TCP,TCP_NODELAY,(char *)&flag,sizeof(flag)) == -1) {
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

	while(1) {
		#if PRINT_NETWORK_DEBUGGING
		printf("Waiting for new connection.\n");
		#endif

		/* Accept actual connection from the client */
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
				                        (socklen_t *)&clilen);

		if (newsockfd < 0)
		{
			perror("ERROR on accept");
	//		    exit(1);
			continue;
		}
		#if PRINT_NETWORK_DEBUGGING
		printf("Got a new connection!\n");
		#endif

		struct timeval tv;
		tv.tv_sec = 10; // 10 second timeout
		tv.tv_usec = 0; // you need to initialize this

		setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

		while(1) {
			#if PRINT_NETWORK_DEBUGGING
		    printf("Waiting for request.\n");
			#endif

			memset(buffer, 0, 256);
			// n = recv( newsockfd,buffer,255,0 );
			n = read(newsockfd, buffer, 256);
			if (n <= 0)
			{
				perror("ERROR reading from socket");
				break;
			}

			#if PRINT_NETWORK_DEBUGGING
			printf("Received a request for the particle data.\n");
			#endif

			/* read the current particle report from "particleReport.yaml" and send it back */
			// char msg[mostRecentVR.numTargetsFound * 24 + 1];
			char msg[2048]; // let's just make this big.

			pthread_mutex_lock( &mostRecentPRMutex );
			int charsWritten = 0;

			#if PRINT_NETWORK_DEBUGGING
			printf("numTargetsFound: %d\n",mostRecentVR.numTargetsFound );
			#endif

			int w;
			for (w = 0; w < mostRecentVR.numTargetsFound; w++)
			{
				// commented out because I changed what's in the Report struct
		  		charsWritten += sprintf(&msg[w*24], "%.3f,%.3f,%.3f,%.3f:", mostRecentVR.targetsFound[w].ctrX, mostRecentVR.targetsFound[w].ctrY, mostRecentVR.targetsFound[w].aspectRatio, mostRecentVR.targetsFound[w].boundingArea);

				#if PRINT_NETWORK_DEBUGGING
				printf("charsWritten: %d\n", charsWritten);
				#endif
			}
			if (mostRecentVR.numTargetsFound == 0) {
				msg[0] = '\0';
			} else {
				// the last one should not have a ':'
				msg[(w)*24 -1] = '\0';
			}

			pthread_mutex_unlock( &mostRecentPRMutex );


			#if PRINT_NETWORK_DEBUGGING
			printf("message to send: %s\n", msg);
			#endif

			n = write(newsockfd,msg,charsWritten);
			if (n < 0)
			{
				perror("ERROR writing to socket");
				//exit(1);
				break;
			}
			#if PRINT_NETWORK_DEBUGGING
			printf("Sent!\n");
			#endif

		} // inner loop
		close(newsockfd);

		#if PRINT_NETWORK_DEBUGGING
		printf("Succesfully closed connection.\n");
		#endif
	} // outer loop
	pthread_exit(0);
}

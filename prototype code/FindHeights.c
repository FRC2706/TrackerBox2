/*
 A Simple OpenCV Tutorial
 by Jean-Marc Pelletier (c)2009
 
 This program identities edges and contours in the camera image. When the user clicks inside a closed path, the area of the contour immediately enclosing the mouse pointer is 
 written out.
 
 The user can start or stop the image by pressing the space bar and quit the application by pressing the "esc" key.
 
 This turorial is meant to show the basic structure of a simple OpenCV application. Furthermore, it shows how one can work with sequences and contours, which are important
 parts of OpenCV that could be better documented.
 
 */

#include "cv.h"
#include "opencv2/opencv.hpp"
#include "highgui.h"
#include <stdio.h>
#include <stdlib.h>

//Useful values
#define MAX_CONTOUR_LEVELS 10 //This will be used when displaying contours

#define DEFAULT_TRACKBAR_VAL 128 //For the trackbars

//Function declarations
CvContour* contourFromPosition(CvContour *contours, int x, int y); //Returns a pointer to the inner-most contour surrounding the given point
char pointIsInsideContour(CvContour *contour, int x, int y); //Returns whether the given position is inside a contour

int main (int argc, const char * argv[]) {
	
	if (argc < 2) {
		printf("Please provide a file to process\n");
		exit(0);
	}

	int thresh1=DEFAULT_TRACKBAR_VAL, thresh2=DEFAULT_TRACKBAR_VAL; //These two variables will hold trackbar positions.
	
	cvNamedWindow("Tutorial", 0); //Here we create a window and give it a name. The second argument tells the window to not automatically adjust its size.
	
	IplImage *frame; //This will point to the IPL image we will retrieve from the camera.
	
	//Grab a frame from the file provided on the command-line.
	frame = cvLoadImage( argv[1] );
	
	if(!frame) exit(0); //Couldn't get an image, try again next time.
	
	int img_width = frame->width;  //Image width
	int img_height = frame->height; //Image height

	//These are pointers to IPL images, which will hold the result of our calculations
	IplImage *small_image = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,3); //size, depth, channels (RGB = 3)
	IplImage *small_grey_image = cvCreateImage(cvGetSize(small_image), IPL_DEPTH_8U, 1); //1 channel for greyscale
	IplImage *edge_image = cvCreateImage(cvGetSize(small_image), IPL_DEPTH_8U, 1); //We use cvGetSize to make sure the images are the same size. 
	
	//CvMemStorage and CvSeq are structures used for dynamic data collection. CvMemStorage contains pointers to the actual
	//allocated memory, but CvSeq is used to access this data. Here, it will hold the list of image contours.
	CvMemStorage *storage = cvCreateMemStorage(0);
	
	//In computer vision, it's always better to work with the smallest images possible, for faster performance.
	//cvResize will use inter-linear interpolation to fit frame into small_image.
	cvResize(frame, small_image, CV_INTER_LINEAR);

	// smooth out the image to remove some of the holes, it also blurs the result.
	cvSmooth(small_image, small_image, CV_MEDIAN, 2*2+1);
	
	//Many computer vision algorithms do not use colour information. Here, we convert from RGB to greyscale before processing further.
	cvCvtColor(small_image, small_grey_image, CV_RGB2GRAY);
	
	//We then detect edges in the image using the Canny algorithm. This will return a binary image, one where the pixel values will be 255 for 
	//pixels that are edges and 0 otherwise. This is unlike other edge detection algorithms like Sobel, which compute greyscale levels.
	cvCanny(small_grey_image, edge_image, (double)thresh1, (double)thresh2, 3); //We use the threshold values from the trackbars and set the window size to 3
	
	//The edges returned by the Canny algorithm might have small holes in them, which will cause some problems during contour detection.
	//The simplest way to solve this problem is to "dilate" the image. This is a morphological operator that will set any pixel in a binary image to 255 (on) 
	//if it has one neighbour that is not 0. The result of this is that edges grow fatter and small holes are filled in.
	//We re-use small_grey_image to store the results, as we won't need it anymore.
	cvDilate(edge_image, small_grey_image, 0, 1);
	
	//Once we have a binary image, we can look for contours in it. cvFindContours will scan through the image and store connected contours in "sorage".
	//"contours" will point to the first contour detected. CV_RETR_TREE means that the function will create a contour hierarchy. Each contour will contain 
	//a pointer to contours that are contained inside it (holes). CV_CHAIN_APPROX_NONE means that all the contours points will be stored. Finally, an offset
	//value can be specified, but we set it to (0,0).
//	CvSeq *contours = 0;
//	cvFindContours(small_grey_image, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
//	cvFindContours(small_grey_image, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
	
	std::vector<std::vector<cv::Point> > contours;
	cv::Mat mat_small_grey_image(small_grey_image);
	cv::Mat mat_small_image(small_image);

	cv::findContours(mat_small_grey_image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	printf("Found %d targets.\n", (int) contours[0].size());

	//This function will display contours on top of an image. We can specify different colours depending on whether the contour in a hole or not.
//	cv::drawContours(mat_small_image, contours, CV_RGB(255,0,0), CV_RGB(0,255,0), MAX_CONTOUR_LEVELS, 1, CV_AA, cvPoint(0,0));


	// find the points on the contour that are closest to the outside conerns of the image.
	std::vector<cv::Point> topLeft(contours.size()); // Top Left Point of Bounding Box
	std::vector<cv::Point> botLeft(contours.size()); // Bottom Left Point of Bounding Box
	std::vector<cv::Point> topRight(contours.size()); // Top Right Point of Bounding Box
	std::vector<cv::Point> botRight(contours.size()); // Bottom Right Point of Bounding Box

	// I'm cheating and using squared distance because I only need find the smallest so I can avoid doing the slow square-root.
	long distTopLeft, distBotLeft, distTopRight, distBotRight;

	// initialize random seed:
	srand ( time(NULL) );
	for (unsigned int i = 0; i < contours.size(); ++i) {
		cv::Scalar colour = cv::Scalar( rand()%255, rand()%255, rand()%255 );
		cv::drawContours(mat_small_image, contours, i, colour, 2);	// draw the contour in a random colour

		// if there are no points in this contour, skip it.
		if(contours[i].size() <= 0)
			continue;

		// if this contour has too small an area, skip it.
		if(contourArea(contours[i]) < 100)
			continue;

		
		// Find the bounding box for this target
		cv::Rect bb;	// the bounding box
		std::vector<cv::Point> contour_poly( contours[i].size() );
		cv::approxPolyDP( cv::Mat( contours[i] ), contour_poly, 3, true );
		bb = boundingRect( cv::Mat(contour_poly) );
		
		// convienience vars to save typing
		int bblx = bb.x;					// x coordinate of the left side of the bounding box
		int bbrx = bb.x + bb.width;	// x coordinate of the right side of the bounding box
		int bbty = bb.y;					// y coordinate of the top of the bounding box
		int bbby = bb.y + bb.height;	// y coordinate of the bottem of the bounding box

		topLeft[i] = botLeft[i] = topRight[i] = botRight[i] = contours[i][0];  // Assumes the contour has at least 1 point

		// these should be relative to the target's basic bounding box
		distTopLeft = (bblx-topLeft[i].x) * (bblx-topLeft[i].x) + (bbty-topLeft[i].y) * (bbty-topLeft[i].y);
		distBotLeft = (bblx-botLeft[i].x) * (bblx-botLeft[i].x) + (bbby-botLeft[i].y) * (bbby-botLeft[i].y);
		distTopRight = (bbrx-topRight[i].x) * (bbrx-topRight[i].x) + (bbty-topRight[i].y) * (bbty-topRight[i].y);
		distBotRight = (bbrx-botRight[i].x) * (bbrx-botRight[i].x) + (bbty-botRight[i].y) * (bbty-botRight[i].y);
		int dist;
		for(unsigned int j = 1; j < contours[i].size(); ++j){
//			dist = contours[i][j].x * contours[i][j].x + contours[i][j].y * contours[i][j].y;
			dist = (bblx-contours[i][j].x) * (bblx-contours[i][j].x) + (bbty-contours[i][j].y) * (bbty-contours[i][j].y);
	      if (dist < distTopLeft){
	         topLeft[i].x = contours[i][j].x;
				topLeft[i].y = contours[i][j].y;
				distTopLeft = dist;
	      }
//			dist = contours[i][j].x * contours[i][j].x + (IMG_HEIGHT-contours[i][j].y) * (IMG_HEIGHT-contours[i][j].y);
			dist = (bblx-contours[i][j].x) * (bblx-contours[i][j].x) + (bbby-contours[i][j].y) * (bbby-contours[i][j].y);
			if (dist < distBotLeft){
	         botLeft[i].x = contours[i][j].x;
				botLeft[i].y = contours[i][j].y;
				distBotLeft = dist;
	      }
//			dist = (IMG_WIDTH-contours[i][j].x) * (IMG_WIDTH-contours[i][j].x) + contours[i][j].y * contours[i][j].y;
			dist = (bbrx-contours[i][j].x) * (bbrx-contours[i][j].x) + (bbty-contours[i][j].y) * (bbty-contours[i][j].y);
			if (dist < distTopRight){
	         topRight[i].x = contours[i][j].x;
				topRight[i].y = contours[i][j].y;
				distTopRight = dist;
	      }
//			dist = (IMG_WIDTH-contours[i][j].x) * (IMG_WIDTH-contours[i][j].x) + (IMG_HEIGHT-contours[i][j].y) * (IMG_HEIGHT-contours[i][j].y);
			dist = (bbrx-contours[i][j].x) * (bbrx-contours[i][j].x) + (bbby-contours[i][j].y) * (bbby-contours[i][j].y);
			if (dist < distBotRight){
	         botRight[i].x = contours[i][j].x;
				botRight[i].y = contours[i][j].y;
				distBotRight = dist;
	      }
		}
		cv::circle(mat_small_image, topLeft[i], 8, colour, -1);
		cv::circle(mat_small_image, botLeft[i], 8, colour, -1);
		cv::circle(mat_small_image, topRight[i], 8, colour, -1);
		cv::circle(mat_small_image, botRight[i], 8, colour, -1);
	}


	//Finally, display the image in the window.
	cvShowImage("Tutorial", small_image);
	cvWaitKey(0);	// wait until the user presses a key before destroying the window and exiting the program.

	//Clean up before quitting.
	cvDestroyAllWindows(); //This function releases all the windows created so far.
	
	//release memory
	cvReleaseMemStorage(&storage);
	
	//Release images
	cvReleaseImage(&small_image); //We pass a pointer to a pointer, because cvReleaseImage will set the image pointer to 0 for us.
	cvReleaseImage(&small_grey_image);
	cvReleaseImage(&edge_image);
	
	return 0; //We're done.
}

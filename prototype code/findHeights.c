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
#include "highgui.h"
#include <stdio.h>

//Useful values
#define MAX_CONTOUR_LEVELS 10 //This will be used when displaying contours

#define DEFAULT_TRACKBAR_VAL 128 //For the trackbars

#define IMG_WIDTH 690  //Image width
#define IMG_HEIGHT 230 //Image height

//Function declarations
void onMouse(int event, int x, int y, int flags, void *params); //This will be called when the user performs a mouse action.
CvContour* contourFromPosition(CvContour *contours, int x, int y); //Returns a pointer to the inner-most contour surrounding the given point
char pointIsInsideContour(CvContour *contour, int x, int y); //Returns whether the given position is inside a contour

int main (int argc, const char * argv[]) {
	
	if (argc < 2) {
		printf("Please provide a file to process\n");
		exit(0);
	}

	int thresh1=DEFAULT_TRACKBAR_VAL, thresh2=DEFAULT_TRACKBAR_VAL; //These two variables will hold trackbar positions.
	
	//These are pointers to IPL images, which will hold the result of our calculations
	IplImage *small_image = cvCreateImage(cvSize(IMG_WIDTH,IMG_HEIGHT),IPL_DEPTH_8U,3); //size, depth, channels (RGB = 3)
	IplImage *small_grey_image = cvCreateImage(cvGetSize(small_image), IPL_DEPTH_8U, 1); //1 channel for greyscale
	IplImage *edge_image = cvCreateImage(cvGetSize(small_image), IPL_DEPTH_8U, 1); //We use cvGetSize to make sure the images are the same size. 
	
	//CvMemStorage and CvSeq are structures used for dynamic data collection. CvMemStorage contains pointers to the actual
	//allocated memory, but CvSeq is used to access this data. Here, it will hold the list of image contours.
	CvMemStorage *storage = cvCreateMemStorage(0);
	CvSeq *contours = 0;
	
	cvNamedWindow("Tutorial", 0); //Here we create a window and give it a name. The second argument tells the window to not automatically adjust its size.
	
	//We add two trackbars (sliders) to the window. These will be used to set the parameters for the Canny edge detection.
	cvCreateTrackbar("Thresh1", "Tutorial", &thresh1, 256, 0);
	cvCreateTrackbar("Thresh2", "Tutorial", &thresh2, 256, 0);
	
	//Set the trackbar position to the default value. 
	cvSetTrackbarPos("Thresh1", "Tutorial", DEFAULT_TRACKBAR_VAL); //Trackbar name, window name, position
	cvSetTrackbarPos("Thresh2", "Tutorial", DEFAULT_TRACKBAR_VAL);

	//Now set the mouse callback function. We need to pass the location of the contours so that it will be able to access this information.
	cvSetMouseCallback("Tutorial", onMouse, &contours); //Window name, function pointer, user-defined parameters.
	IplImage *frame; //This will point to the IPL image we will retrieve from the camera.
	
	//Grab a frame from the file provided on the command-line.
	frame = cvLoadImage( argv[1] );
	
	if(!frame) exit(0); //Couldn't get an image, try again next time.
	
	//In computer vision, it's always better to work with the smallest images possible, for faster performance.
	//cvResize will use inter-linear interpolation to fit frame into small_image.
	cvResize(frame, small_image, CV_INTER_LINEAR);

	cvSmooth(small_image, small_image, CV_MEDIAN, 2*3+1);
	
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
	cvFindContours(small_grey_image, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
	
	//This function will display contours on top of an image. We can specify different colours depending on whether the contour in a hole or not.
	cvDrawContours(small_image, contours, CV_RGB(255,0,0), CV_RGB(0,255,0), MAX_CONTOUR_LEVELS, 1, CV_AA, cvPoint(0,0));
	
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


//When the user clicks in the window...
void onMouse(int event, int x, int y, int flags, void *params){
	//params points to the contour sequence. Here we cast the pointer to CvContour instead of CvSeq.
	//CvContour is in fact an extension of CvSeq and is the structure used by cvFindContours, if we cast 
	//to CvSeq, we won't be able to access the fields specific to CvContour.
	CvContour *contours = *(CvContour **)params;
	CvContour *ctr; //This will point to the specific contour the user clicked in.
	double area; //This will hold the area of the contour the user clicked in.
	
	if(!contours)return; //If there are no contours, we don't bother doing anything.
	
	//"event" tells us what occured
	switch(event){
		case CV_EVENT_LBUTTONDOWN: //single click
		case CV_EVENT_LBUTTONDBLCLK: //double click
			printf("Click: %d %d\n",x,y);  //Write out where the user clicked.
			
			//Here we retrieve the inner-most contour the user clicked in. If there is no such contour,
			//the function returns a null pointer, which we need to check against.
			ctr = contourFromPosition(contours, x, y);
			if(ctr){ //The user did click inside something
				//Calculate the area of the contour using cvContourArea. CV_WHOLE_SEQ means we want the area of the whole contour. 
				//Important: we need to calculate the absolute value of the result using fabs because cvContourArea can return negative values.
				area = fabs(cvContourArea(ctr, CV_WHOLE_SEQ));
				printf("Area: %f\n",area); //and print the result out.
			}
			
			break;
		case CV_EVENT_LBUTTONUP: //single click up
		case 10: //double-click up? (not documented)
			break;
	}
}

CvContour* contourFromPosition(CvContour *contours, int x, int y){
	CvContour *contour; //This will point to a single contour.
	CvContour *tmpContour=0, *output=0; //Two more pointers, first a temporary contour and the final output.
	
	if(!contours)return 0; //Don't bother doing anything if there are no contours.
	
	contour = contours; //Now contour points to the first contour in the list
	
	while(contour){ //We loop through the list of contours...
		if(pointIsInsideContour(contour, x, y)){ //Check to see if the point the user clicked on is inside the current contour, if yes...
			output = contour; //Since the point is inside the contour, set the output accordingly.
			
			//We now need to check all the contours inside the one we are processing. We go down one level by following the v_next (vertical next) pointer in the 
			//CvSeq/CvContour structure. From then, we recurse into this function to go through all the contours.
			if(contour->v_next){
				//It's possible that this will return a null pointer, so we store the results in a temporary pointer...
				tmpContour = contourFromPosition((CvContour *)contour->v_next,x,y);
				//and only update the output if the pointer is valid.
				if(tmpContour)output = tmpContour;
				
				//At this point, because the sequence is organized as a tree, we know that output points to the inner-most contour, which is what we want.
			}
		}
		
		//Once we're done with this contour, we need to move to the following contour that's not included in the one we just processed.
		//This is pointed to by the h_next (horizontal next) pointer. 
		contour = (CvContour *)(contour->h_next);
	}
	
	return output;
}

//This boolean function returns whether the (x,y) point is inside the given contour.
char pointIsInsideContour(CvContour *contour, int x, int y){
	//We know that a point is inside a contour if we can find one point in the contour that is immediately to the left,
	//one that is immediately on top, one that is immediately to the right and one that is immediately below the (x,y) point.
	//We will update the boolean variables below when these are found:
	char found_left=0, found_top=0, found_right=0, found_bottom=0;
	int count, i; //Variables used for iteration
	CvPoint *contourPoint; //A pointer to a single contour poit.
	
	if(!contour)return 0; //Don't bother doing anything if there is no contour.
	
	count = contour->total; //The total field holds the number of points in the contour.
	
	for(i=0;i<count;i++){ //So, for every point in the contour...
		//We retrieve a pointer to the point at index i using the useful macro CV_GET_SEQ_ELEM
		contourPoint = (CvPoint *)CV_GET_SEQ_ELEM(CvPoint,contour,i);
		
		if(contourPoint->x == x){ //If the point is on the same vertical plane as (x,y)...
			if(contourPoint->y < y)found_top = 1; //and is above (x,y), we found the top.
			else found_bottom = 1; //Otherwise, it's the bottom.
		}
		if(contourPoint->y == y){ //Do the same thing for the horizontal axis...
			if(contourPoint->x < x)found_left = 1;
			else found_right = 1;
		}
	}
	
	return found_left && found_top && found_right && found_bottom; //Did we find all four points?
}


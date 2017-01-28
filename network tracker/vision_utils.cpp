#include "vision_utils.h"


// TODO: have this return the values. This will require making some new structs.
VisionReport findFRCVisionTargets(IplImage* mask, IplImage* outputImage, int minTargetArea) {

	//These are pointers to IPL images, which will hold the result of our calculations
	IplImage *working_image = cvCreateImage(cvGetSize(outputImage), IPL_DEPTH_8U, 1); //1 channel for greyscale
	IplImage *edge_image = cvCreateImage(cvGetSize(outputImage), IPL_DEPTH_8U, 1); //We use cvGetSize to make sure the images are the same size.

	//We then detect edges in the image using the Canny algorithm. This will return a binary image, one where the pixel values will be 255 for
	//pixels that are edges and 0 otherwise. This is unlike other edge detection algorithms like Sobel, which compute greyscale levels.
	cvCanny(mask, edge_image, (double)128, (double)128, 3); //We use the threshold values from the trackbars and set the window size to 3

	//The edges returned by the Canny algorithm might have small holes in them, which will cause some problems during contour detection.
	//The simplest way to solve this problem is to "dilate" the image. This is a morphological operator that will set any pixel in a binary image to 255 (on)
	//if it has one neighbour that is not 0. The result of this is that edges grow fatter and small holes are filled in.
	//We re-use working_image to store the results, as we won't need it anymore.
    cvDilate(edge_image, working_image, 0, 1);

	//Once we have a binary image, we can look for contours in it. cvFindContours will scan through the image and store connected contours in "sorage".
	//"contours" will point to the first contour detected. CV_RETR_TREE means that the function will create a contour hierarchy. Each contour will contain
	//a pointer to contours that are contained inside it (holes). CV_CHAIN_APPROX_NONE means that all the contours points will be stored. Finally, an offset
	//value can be specified, but we set it to (0,0).
	std::vector<std::vector<cv::Point> > contours;
	cv::Mat mat_working_image(working_image);
	cv::Mat mat_outputImage(outputImage);
	cv::findContours(mat_working_image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// find the points on the contour that are closest to the outside conerns of the image.
	std::vector<cv::Point> topLeft(contours.size()); // Top Left Point of Bounding Box
	std::vector<cv::Point> botLeft(contours.size()); // Bottom Left Point of Bounding Box
	std::vector<cv::Point> topRight(contours.size()); // Top Right Point of Bounding Box
	std::vector<cv::Point> botRight(contours.size()); // Bottom Right Point of Bounding Box

	// I'm cheating and using squared distance because I only need find the smallest so I can avoid doing the slow square-root.
	long distTopLeft, distBotLeft, distTopRight, distBotRight;

	//cv::Scalar drawColour = cv::Scalar( 44, 169, 62 ); // use a random drawColour to tell each target apart
	cv::Scalar drawColour = cv::Scalar( 237, 19, 75 ); // use a random drawColour to tell each target apart

	// initialize random seed:
	srand ( time(NULL) );
	unsigned int numTargetsFound=0;
	for (unsigned int i = 0; i < contours.size(); ++i) {

		// if there are no points in this contour, skip it.
		if(contours[i].size() <= 0)
			continue;

		// if this contour has too small an area, skip it.
		if(contourArea(contours[i]) < minTargetArea)
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

		topLeft.push_back(contours[i][0]);
		botLeft.push_back(contours[i][0]);
		topRight.push_back(contours[i][0]);
		botRight.push_back(contours[i][0]);

		// these should be relative to the target's basic bounding box
		distTopLeft = (bblx-topLeft[numTargetsFound].x) * (bblx-topLeft[numTargetsFound].x) + (bbty-topLeft[numTargetsFound].y) * (bbty-topLeft[numTargetsFound].y);
		distBotLeft = (bblx-botLeft[numTargetsFound].x) * (bblx-botLeft[numTargetsFound].x) + (bbby-botLeft[numTargetsFound].y) * (bbby-botLeft[numTargetsFound].y);
		distTopRight = (bbrx-topRight[numTargetsFound].x) * (bbrx-topRight[numTargetsFound].x) + (bbty-topRight[numTargetsFound].y) * (bbty-topRight[numTargetsFound].y);
		distBotRight = (bbrx-botRight[numTargetsFound].x) * (bbrx-botRight[numTargetsFound].x) + (bbty-botRight[numTargetsFound].y) * (bbty-botRight[numTargetsFound].y);
		int dist;
		for(unsigned int j = 1; j < contours[i].size(); ++j){
			dist = (bblx-contours[i][j].x) * (bblx-contours[i][j].x) + (bbty-contours[i][j].y) * (bbty-contours[i][j].y);
		  if (dist < distTopLeft){
				topLeft[numTargetsFound].x = contours[i][j].x;
				topLeft[numTargetsFound].y = contours[i][j].y;
				distTopLeft = dist;
		  }
			dist = (bblx-contours[i][j].x) * (bblx-contours[i][j].x) + (bbby-contours[i][j].y) * (bbby-contours[i][j].y);
			if (dist < distBotLeft){
				botLeft[numTargetsFound].x = contours[i][j].x;
				botLeft[numTargetsFound].y = contours[i][j].y;
				distBotLeft = dist;
		  }
			dist = (bbrx-contours[i][j].x) * (bbrx-contours[i][j].x) + (bbty-contours[i][j].y) * (bbty-contours[i][j].y);
			if (dist < distTopRight){
				topRight[numTargetsFound].x = contours[i][j].x;
				topRight[numTargetsFound].y = contours[i][j].y;
				distTopRight = dist;
		  }
			dist = (bbrx-contours[i][j].x) * (bbrx-contours[i][j].x) + (bbby-contours[i][j].y) * (bbby-contours[i][j].y);
			if (dist < distBotRight){
				botRight[numTargetsFound].x = contours[i][j].x;
				botRight[numTargetsFound].y = contours[i][j].y;
				distBotRight = dist;
		  }
		}

	/******** DRAW STUFF ONTO THE OUTPUT IMAGE ********/

	// draw the corner points
	cv::circle(mat_outputImage, topLeft[numTargetsFound], 8, drawColour, -1);
	cv::circle(mat_outputImage, botLeft[numTargetsFound], 8, drawColour, -1);
	cv::circle(mat_outputImage, topRight[numTargetsFound], 8, drawColour, -1);
	cv::circle(mat_outputImage, botRight[numTargetsFound], 8, drawColour, -1);

	// set the x coordinates for the bottom points to match the top points since we don't actually care about the X anyways.
	botLeft[numTargetsFound].x = topLeft[numTargetsFound].x;
	botRight[numTargetsFound].x = topRight[numTargetsFound].x;
	cv::drawContours(mat_outputImage, contours, i, drawColour, 2);	// draw the outline of the object
	cv::line(mat_outputImage, topLeft[numTargetsFound], botLeft[numTargetsFound], drawColour, 10);
	cv::line(mat_outputImage, topRight[numTargetsFound], botRight[numTargetsFound], drawColour, 10);

		// commented out so I can display the area instead.
		// disaplay the skew as a ratio of the height of the left and right sides.
		// print the text sorta centred below the bottom of the target.
		float Lheight = (botLeft[i].y - topLeft[i].y);
		float Rheight = (botRight[i].y - topRight[i].y);
		float Twidth = (topRight[i].x - topLeft[i].x);
		float Bwidth = (botRight[i].x - botLeft[i].x);

		char text[16];
		sprintf(text, "%.3f", ((Rheight+Lheight)/2)/((Twidth+Bwidth)/2));
		// sprintf(text, "%.1f", contourArea(contours[i]));
		cv::Point textLoc( (botLeft[i].x + botRight[i].x)/2, (botLeft[i].y + botRight[i].y)/2 + 30);
		cv::putText( mat_outputImage, text, textLoc, CV_FONT_HERSHEY_COMPLEX, 0.75, drawColour);

		numTargetsFound++;
	}


	// Bundle the data into structs to hand back to the roboRIO.

	VisionReport vr;

	vr.numTargetsFound = numTargetsFound;
	vr.targetsFound = new VisionTarget[numTargetsFound];

	for (unsigned int i = 0; i < numTargetsFound; i++)
	{
		float Lheight = (botLeft[i].y - topLeft[i].y);
		float Rheight = (botRight[i].y - topRight[i].y);
		float Twidth = (topRight[i].x - topLeft[i].x);
		float Bwidth = (botRight[i].x - botLeft[i].x);

		vr.targetsFound[i].aspectRatio = ((Rheight + Lheight) / 2) / ((Twidth + Bwidth) / 2);

		// size of the image
		// mask->width, mask->height
		int meanXpx = (botLeft[i].x + botRight[i].x) / 2;
		int meanYpx = (((topLeft[i].y + botLeft[i].y) / 2) + ((topRight[i].y + botRight[i].y) / 2)) / 2;

		vr.targetsFound[i].ctrX = (meanXpx - ((float)mask->width / 2)) / ((float)mask->width / 2);
		vr.targetsFound[i].ctrY = (meanYpx - ((float)mask->height / 2)) / ((float)mask->height / 2);

		vr.targetsFound[i].boundingArea = (((Lheight + Rheight) / 2) * (Bwidth)) / ((float)mask->width * (float)mask->height);

		// draw the centre dot
		cv::circle(mat_outputImage, cv::Point(meanXpx, meanYpx), 13, drawColour, -1);
	}

  // Free the memory for the images we don't need to keep.
  cvReleaseImage(&working_image);
  cvReleaseImage(&edge_image);

	return vr;
}
////////////////////////////////////////////////////////
///2017 First Steamworks code starts here!//////////////
////////////////////////////////////////////////////////
/** Given a binary image, find the centre of mass of the white pixels **/
VisionReport findCOM(IplImage* mask, IplImage* outputImage, int minTargetArea) {

	cv::Mat mat_outputImage(outputImage);
	cv::Scalar drawColour = cv::Scalar( 237, 19, 75 );

    // compute COM of the mask!
    int xAccum=0, yAccum=0, areaAccum=0;
	for (int i = 0; i < mask->width; i++) {
		for (int j = 0; j < mask->height; j++) {
    		// if this pixel is a 1
    		if (mask->imageData[mask->widthStep * j + i]) {  // this will be 0 if black, and -1 if white.
    			areaAccum++;  // add 1 to the count of white pixels.
    			xAccum += i;  // add the x-coordinate to a running total
    			yAccum += j;  // add the y-coordinate to a running total
            }
		}
    }


    VisionReport vr;

    // if there is very little white, return no targets found.
    if(areaAccum <= minTargetArea)
    {
        vr.numTargetsFound = 0;
        return vr;
    }
    // else process the entire image as one target.

    vr.numTargetsFound = 1;
	vr.targetsFound = new VisionTarget[1];

	// The centre of mass of the target is essentially the "average" pixel.
    int meanXpx=0, meanYpx=0;
    if(areaAccum != 0) {
    	meanXpx = xAccum / areaAccum;
    	meanYpx = yAccum / areaAccum;
    }
    // Now convert it from pixel coordinates to [-1,1]
	vr.targetsFound[0].ctrX = (meanXpx - ((float)mask->width / 2)) / ((float)mask->width / 2);
	vr.targetsFound[0].ctrY = (meanYpx - ((float)mask->height / 2)) / ((float)mask->height / 2);

    // area of the image occupied by the target (ie by white pixels).
    vr.targetsFound[0].boundingArea = ((float) areaAccum) / (mask->width * mask->height);

    // Draw on he display image.
    cv::circle(mat_outputImage, cv::Point(meanXpx, meanYpx), 13, drawColour, -1);


  return vr;
}

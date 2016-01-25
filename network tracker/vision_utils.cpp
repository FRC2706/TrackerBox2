#include "vision_utils.h"



// TODO: have this return the values. This will require making some new structs.
void findFRCVisionTargets(IplImage* mask, IplImage* outputImage) {

  int thresh1=128, thresh2=128;

	//These are pointers to IPL images, which will hold the result of our calculations
  //	IplImage *outputImage = cvCreateImage(cvSize(img_width,img_height),IPL_DEPTH_8U,3); //size, depth, channels (RGB = 3)
	IplImage *working_image = cvCreateImage(cvGetSize(outputImage), IPL_DEPTH_8U, 1); //1 channel for greyscale
	IplImage *edge_image = cvCreateImage(cvGetSize(outputImage), IPL_DEPTH_8U, 1); //We use cvGetSize to make sure the images are the same size.

	//In computer vision, it's always better to work with the smallest images possible, for faster performance.
	//cvResize will use inter-linear interpolation to fit mask into outputImage (matching the size of the image they gave us).
//	cvResize(mask, working_image, CV_INTER_LINEAR);

	// smooth out the image to remove some of the holes, it also blurs the result.
//	cvSmooth(outputImage, outputImage, CV_MEDIAN, 2*2+1);

	//Many computer vision algorithms do not use colour information. Here, we convert from RGB to greyscale before processing further.
//	cvCvtColor(mask, working_image, CV_RGB2GRAY);

	//We then detect edges in the image using the Canny algorithm. This will return a binary image, one where the pixel values will be 255 for
	//pixels that are edges and 0 otherwise. This is unlike other edge detection algorithms like Sobel, which compute greyscale levels.
	cvCanny(mask, edge_image, (double)thresh1, (double)thresh2, 3); //We use the threshold values from the trackbars and set the window size to 3

	//The edges returned by the Canny algorithm might have small holes in them, which will cause some problems during contour detection.
	//The simplest way to solve this problem is to "dilate" the image. This is a morphological operator that will set any pixel in a binary image to 255 (on)
	//if it has one neighbour that is not 0. The result of this is that edges grow fatter and small holes are filled in.
	//We re-use working_image to store the results, as we won't need it anymore.
	cvDilate(edge_image, working_image, 0, 1);

	//Once we have a binary image, we can look for contours in it. cvFindContours will scan through the image and store connected contours in "sorage".
	//"contours" will point to the first contour detected. CV_RETR_TREE means that the function will create a contour hierarchy. Each contour will contain
	//a pointer to contours that are contained inside it (holes). CV_CHAIN_APPROX_NONE means that all the contours points will be stored. Finally, an offset
	//value can be specified, but we set it to (0,0).
//	CvSeq *contours = 0;
//	cvFindContours(working_image, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
//	cvFindContours(working_image, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));

	std::vector<std::vector<cv::Point> > contours;
	cv::Mat mat_working_image(working_image);
	cv::Mat mat_outputImage(outputImage);

	cv::findContours(mat_working_image, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
  if (contours.size() > 0)
	 printf("Found %d targets.\n", (int) contours[0].size());
  else
    printf("Found 0 targets.\n");

	//This function will display contours on top of an image. We can specify different colours depending on whether the contour in a hole or not.
//	cv::drawContours(mat_outputImage, contours, CV_RGB(255,0,0), CV_RGB(0,255,0), MAX_CONTOUR_LEVELS, 1, CV_AA, cvPoint(0,0));


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
		//cv::Scalar colour = cv::Scalar( rand()%255, rand()%255, rand()%255 ); // use a random colour to tell each target apart
    cv::Scalar colour = cv::Scalar( 44, 169, 62 ); // use a random colour to tell each target apart

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
			dist = (bblx-contours[i][j].x) * (bblx-contours[i][j].x) + (bbty-contours[i][j].y) * (bbty-contours[i][j].y);
	      if (dist < distTopLeft){
	         topLeft[i].x = contours[i][j].x;
				topLeft[i].y = contours[i][j].y;
				distTopLeft = dist;
	      }
			dist = (bblx-contours[i][j].x) * (bblx-contours[i][j].x) + (bbby-contours[i][j].y) * (bbby-contours[i][j].y);
			if (dist < distBotLeft){
	         botLeft[i].x = contours[i][j].x;
				botLeft[i].y = contours[i][j].y;
				distBotLeft = dist;
	      }
			dist = (bbrx-contours[i][j].x) * (bbrx-contours[i][j].x) + (bbty-contours[i][j].y) * (bbty-contours[i][j].y);
			if (dist < distTopRight){
	         topRight[i].x = contours[i][j].x;
				topRight[i].y = contours[i][j].y;
				distTopRight = dist;
	      }
			dist = (bbrx-contours[i][j].x) * (bbrx-contours[i][j].x) + (bbby-contours[i][j].y) * (bbby-contours[i][j].y);
			if (dist < distBotRight){
	         botRight[i].x = contours[i][j].x;
				botRight[i].y = contours[i][j].y;
				distBotRight = dist;
	      }
		}
		// draw the corner points
		//cv::circle(mat_outputImage, topLeft[i], 8, colour, -1);
		//cv::circle(mat_outputImage, botLeft[i], 8, colour, -1);
		//cv::circle(mat_outputImage, topRight[i], 8, colour, -1);
		//cv::circle(mat_outputImage, botRight[i], 8, colour, -1);

    // set the x coordinates for the bottom points to match the top points since we don't actually care about the X anyways.
		botLeft[i].x = topLeft[i].x;
		botRight[i].x = topRight[i].x;
		cv::drawContours(mat_outputImage, contours, i, colour, 1);	// draw the outline of the object
		cv::line(mat_outputImage, topLeft[i], botLeft[i], colour, 10);
		cv::line(mat_outputImage, topRight[i], botRight[i], colour, 10);

		// disaplay the skew as a ratio of the height of the left and right sides.
		// print the text sorta centred below the bottom of the target.
		char text[16];
		sprintf(text, "%.3f", ((float)topLeft[i].y - botLeft[i].y) / (topRight[i].y - botRight[i].y) );
		cv::Point textLoc( (botLeft[i].x + botRight[i].x)/2, (botLeft[i].y + botRight[i].y)/2 + 30);
		cv::putText( mat_outputImage, text, textLoc, CV_FONT_HERSHEY_COMPLEX, 0.75, colour);
	}

  // free the memory for all the images
  cvReleaseImage(&working_image);
  cvReleaseImage(&edge_image);

//  outputImage = mat_outputImage;
}

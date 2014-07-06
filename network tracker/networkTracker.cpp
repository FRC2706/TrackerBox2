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

#include "networkTracker.h"
#include "networkTrackerYAML_utils.h"

#include <highgui.h>
#include <fstream>
#include <cstring>      // Needed for memset
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>      // Needed for the socket functions
#include <sys/time.h>

#include "yaml-cpp/yaml.h"


using namespace cv;
using namespace std;

// cRIO_IP 10.2.96.2

Parameters p;
ProfileParameters activeProfile;
int activeProfileSlider = 0;

void writeParams(int x) { 
	p.profiles[p.activeProfileIdx] = activeProfile;
	writeParametersToFile(p);  
}

void updateTrackbars() {
	cvSetTrackbarPos("minH", "Binary Mask", activeProfile.minH);
	cvSetTrackbarPos("maxH", "Binary Mask", activeProfile.maxH);
	cvSetTrackbarPos("size of noise filter pass", "Binary Mask", activeProfile.noiseFilterSize);
	cvSetTrackbarPos("size of smoother pass", "Binary Mask", activeProfile.smootherSize);
	cvSetTrackbarPos("Profile #", "Binary Mask", p.activeProfileIdx);
}

void changeProfile(int x) {
	p.profiles[p.activeProfileIdx] = activeProfile;
	p.activeProfileIdx = activeProfileSlider;
	activeProfile = p.profiles[p.activeProfileIdx];
	
	writeParametersToFile(p);
	updateTrackbars();
}



void loadParams() {
	p = loadParametersFromFile();
	activeProfile = p.profiles[p.activeProfileIdx];
	activeProfileSlider = p.activeProfileIdx;
	updateTrackbars();
}

CvPoint COM_center;

//int CANNY_thresh = 0;
//int circleQualityThresh = 0;

int main( int argc, char** argv )
{
	cvNamedWindow("Binary Mask", CV_WINDOW_AUTOSIZE);
    cvCreateTrackbar( "minH", "Binary Mask", &activeProfile.minH, 255, writeParams);
    cvCreateTrackbar( "maxH", "Binary Mask", &activeProfile.maxH, 255, writeParams);
    cvCreateTrackbar( "size of noise filter pass", "Binary Mask", &activeProfile.noiseFilterSize, 25, writeParams);
    cvCreateTrackbar( "size of smoother pass", "Binary Mask", &activeProfile.smootherSize, 25, writeParams);
    cvCreateTrackbar( "Profile #", "Binary Mask", &activeProfileSlider, 9, changeProfile);
    
	loadParams();

	IplImage* frame;
	CvCapture* capture;

	printf("Connecing to Axis Cam at %s...", p.ipParams.axisCamAddr.c_str());
	cout.flush();
	capture = cvCaptureFromFile(p.ipParams.axisCamAddr.c_str());
//	capture = cvCaptureFromFile("vid.avi");
//	capture = cvCaptureFromCAM(0); // laptop's webcam
	printf("Done!\n\n\n");
	
	cvNamedWindow("Raw Image", CV_WINDOW_AUTOSIZE);
	


	frame = cvQueryFrame( capture );
	
	IplImage* mask = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
	
	ParticleReport report;

	timeval start, ends;
	gettimeofday(&start, 0);
	while(1) {
		frame = cvQueryFrame( capture );
		
		loadParams();
		
		gettimeofday(&ends, 0);
		cout << "FPS: " << 1.0 / ( (double) (ends.tv_sec - start.tv_sec) + (double) (ends.tv_usec - start.tv_usec) / 1000000) << endl;
		start = ends;
		
		cvShowImage("Raw Image", frame);
		
		// Do some processing on the image
		
		mask = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
		
		thresholdHSV(frame, mask, activeProfile.minH, activeProfile.maxH, 40, 255, 40, 255);
		
		smoothImage(mask);
		
//		cvShowImage("Binary Mask", mask);
		
		// compute the center of mass of the target we found
		report = computeParticleReport(mask, report);
		
		// Now maybe draw a dot and arrow for the COM and vel
		IplImage* maskPlusCOM = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 3);
		cvCvtColor(mask, maskPlusCOM, CV_GRAY2BGR);
		
		cvCircle(maskPlusCOM, COM_center, 15, CV_RGB(0,230,40), -1);
		
		cvShowImage("Binary Mask", maskPlusCOM);

		cvReleaseImage(&maskPlusCOM);
		cvReleaseImage(&mask);
		cvWaitKey(5);
	} // video frame loop
	
	cvReleaseImage(&mask);
}

/**
 * This expects a binary mask. It'do weird things if given a 3-channel image.
 */
ParticleReport computeParticleReport(IplImage* mask, ParticleReport prevReport) {
	ParticleReport pr;
	
	int xAccum, yAccum, areaAccum;
	xAccum = yAccum = areaAccum = 0;
	
	for (int i = 0; i < mask->width; i++)
		for (int j = 0; j < mask->height; j++)
			if (mask->imageData[mask->widthStep * j + i]) {
				areaAccum++;
				xAccum += i;
				yAccum += j;
			}	
	
	// average
	COM_center.x = pr.centerX = ((double) xAccum) / areaAccum;
	COM_center.y = pr.centerY = ((double) yAccum) / areaAccum;
	
	// normalize to [-1, 1]
	pr.centerX = (( 2*pr.centerX / mask->width) - 1);
	pr.centerY = (( 2*pr.centerY / mask->height) - 1);
	pr.area = ((double) areaAccum) / (mask->width*mask->height);
	
	// smooth a little bit
	float alpha = 0.4;
	pr.velX = alpha*(pr.centerX - prevReport.centerX) + (1-alpha)*prevReport.velX;
	pr.velY = alpha*(pr.centerY - prevReport.centerY) + (1-alpha)*prevReport.velY;
	
	// check for NANs
	if(isnan(pr.centerX)) pr.centerX = 0.0;
	if(isnan(pr.centerY)) pr.centerY = 0.0;
	if(isnan(pr.area)) pr.area = 0.0;
	if(isnan(pr.velX)) pr.velX = 0.0;
	if(isnan(pr.velY)) pr.velY = 0.0;
	
	
	writeParticleReportToFile(pr);
	
	return pr;
}

void smoothImage(IplImage* image) {
    // To smooth perform a dilation, then an equal and opposite erosion
    
	cvSmooth(image, image, CV_MEDIAN, 2*activeProfile.noiseFilterSize+1);
    
    Mat element = getStructuringElement( MORPH_ELLIPSE,
                                       Size( 2*activeProfile.smootherSize + 1, 2*activeProfile.smootherSize+1 ),
                                       Point( activeProfile.smootherSize, activeProfile.smootherSize ) );
	
	Mat	matImage = Mat(image);
	// Apply the erosion operation
	erode( matImage, matImage, element );
	
	dilate( matImage, matImage, element );
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

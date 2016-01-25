#include "vision_utils.h"

#include "YAML_utils.cpp"

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

	// compute COM of the mask!
	for (int i = 0; i < mask->width; i++)
		for (int j = 0; j < mask->height; j++)
			// if this pixel is a 1
			if (mask->imageData[mask->widthStep * j + i]) {
				areaAccum++;
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

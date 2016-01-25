#ifndef VISION_UTILS__H
#define VISION_UTILS__H

#include "networkTracker.h"

/***** Computer vision stuff *****/

void computeParticleReport(IplImage* mask);
void sendDataTocRIO(const char* msg);
void smoothImage(IplImage* image);
void thresholdHSV(IplImage* image, IplImage* mask, unsigned char minH, unsigned char maxH, unsigned char minS, unsigned char maxS, unsigned char minV, unsigned char maxV);

#endif

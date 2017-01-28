#ifndef VISION_UTILS__H
#define VISION_UTILS__H

#include "vision_data.h"
#include "networkTracker.h"

VisionReport findFRCVisionTargets(IplImage* frame, IplImage* outputImage, int minTargetArea);
VisionReport findCOM(IplImage* mask, IplImage* outputImage, int minTargetArea);

#endif

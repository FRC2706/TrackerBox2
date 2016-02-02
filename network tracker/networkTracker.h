/**
 * Written for the FIRST Robotics Competition
 * Copyright 2016 Mike Ounsworth
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

#ifndef NETWORKTRACKER_H
#define NETWORKTRACKER_H

#include <cv.h>

// let's have wget fetch the image from the camera to the system ramdisk so we don't wear out the SD card with a ton of writes.
#define WGET_PIC_LOC "/dev/shm/camera.jpg"
#define MASK_FILE "/dev/shm/mask.jpg"
#define OUTPUT_FILE "/dev/shm/output.jpg"

/***** structs for holding network data, and results of computing a frame *****/

struct Parameters {
	std::string ipCamAddr;

	// HSV params
	int minHue;
	int maxHue;
	int minSat;
	int maxSat;
	int minVal;
	int maxVal;

	int erodeDilateSize;

	int minTargetArea;
};

/***** Computer vision stuff *****/

void computeParticleReport(IplImage* mask);
void sendDataTocRIO(const char* msg);
void smoothImage(IplImage* image);
void thresholdHSV(IplImage* image, IplImage* mask, unsigned char minH, unsigned char maxH, unsigned char minS, unsigned char maxS, unsigned char minV, unsigned char maxV);


/***** Network stuff to communicate with the roboRIO *****/

#define SOCK_DATA_REQUEST_PORT 1182 // read this from a config file, or hard-code it?

void *runDataRequestServer(void *placeHolder);


#endif

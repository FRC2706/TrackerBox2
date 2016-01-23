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

#ifndef NETWORKTRACKER_H
#define NETWORKTRACKER_H

#include <iostream>
#include <fstream>
//#include <cv.h>
#include <cv.h>
#include "yaml-cpp/yaml.h"

/***** structs for holding network data, and results of computing a frame *****/

struct IPParameters {
	std::string axisCamAddr;
	std::string cRIO_IP;
	std::string cRIO_port;
};

struct ProfileParameters {
	int minH;
	int maxH;
	int noiseFilterSize;
	int smootherSize; // Do I still use this?
};

struct Parameters {
	IPParameters ipParams;
	
	int activeProfileIdx;	
	ProfileParameters profiles[10];
};

struct ParticleReport {
	double centerX; // % of screen
	double centerY; // % of screen
	double area; // % of screen
	double velX;
	double velY;
};


/***** Computer vision stuff *****/

void computeParticleReport(IplImage* mask);
void sendDataTocRIO(const char* msg);
void smoothImage(IplImage* image);
void thresholdHSV(IplImage* image, IplImage* mask, unsigned char minH, unsigned char maxH, unsigned char minS, unsigned char maxS, unsigned char minV, unsigned char maxV);


/***** Network stuff to communicate with the cRIO *****/

#define MY_IP "127.0.0.1" // find this out programatically?
#define SOCK_CHANGE_PROFILE_PORT 1181 // read this from a config file, or hard-code it?
#define SOCK_CHANGE_DATA_REQUEST_PORT 1182 // read this from a config file, or hard-code it?

void *runChangeProfileServer(void *placeHolder);
void *runDataRequestServer(void *placeHolder);


#endif

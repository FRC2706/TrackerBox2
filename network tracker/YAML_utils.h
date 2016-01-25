/**
 * Written for the FIRST Robotics Competition
 * Copyright 2014 Mike Ounsworth
 * ounsworth@gmail.com
 *
 * YAML is a human friendly data serialization standard for all programming languages.
 * We use YAML to save files with parameters and output data from each frame.
 * This file provides utilities for our program to read and write YAML files.
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

#ifndef YAML_UTILS_H
#define YAML_UTILS_H

#include <fstream>
#include <iostream>
#include "yaml-cpp/yaml.h"

#include "networkTracker.h"

/** These operators make reading and comparing the parameters into 1-line operations,
 * rather than having to check each member variable. **/
void operator >> (const YAML::Node& node, IPParameters& p);
void operator >> (const YAML::Node& node, ProfileParameters& p);
bool operator == (Parameters& p1, Parameters& p2);
bool operator != (Parameters& p1, Parameters& p2);
bool operator == (IPParameters& ip1, IPParameters& ip2);
bool operator != (IPParameters& ip1, IPParameters& ip2);
bool operator == (ProfileParameters& pp1, ProfileParameters& pp2);
bool operator != (ProfileParameters& pp1, ProfileParameters& pp2);

#define ramdiskParamsFile "/dev/shm/TrackerBox2_parameters.yaml"
#define permanentParamsFile "TrackerBox2_parameters.yaml"
#define ramdiskPartReportFile "/dev/shm/TrackerBox2_particleReport.yaml"

/** All files are written to and read from RAMDISK at /dev/shm
 * (this is to protect the sd card from wearing out from over-use).
 * The exception to this is the Parameters File which is written to both ramdisk and the sd card each time
 * so that the paremeters persist between shutdowns. **/
Parameters loadParametersFromFile();
void writeParametersToFile(Parameters p);
ParticleReport loadParticleReportFromFile();
void writeParticleReportToFile(ParticleReport p);

#endif

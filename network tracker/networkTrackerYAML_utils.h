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

#ifndef NETWORK_TRACKER_YAML_UTILS_H
#define NETWORK_TRACKER_YAML_UTILS_H

#include <fstream>
#include <iostream>
#include "yaml-cpp/yaml.h"

#include "networkTracker.h"

void operator >> (const YAML::Node& node, IPParameters& p);
void operator >> (const YAML::Node& node, ProfileParameters& p);
bool operator == (Parameters& p1, Parameters& p2);
bool operator != (Parameters& p1, Parameters& p2);
bool operator == (IPParameters& ip1, IPParameters& ip2);
bool operator != (IPParameters& ip1, IPParameters& ip2);
bool operator == (ProfileParameters& pp1, ProfileParameters& pp2);
bool operator != (ProfileParameters& pp1, ProfileParameters& pp2);

Parameters loadParametersFromFile();
void writeParametersToFile(Parameters p);
ParticleReport loadParticleReportFromFile();
void writeParticleReportToFile(ParticleReport p);

#endif

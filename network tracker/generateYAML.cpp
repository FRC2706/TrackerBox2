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


/** I don't remember what this is supposed to do, remove it? **/

#include <fstream>
#include <iostream>
#include "yaml-cpp/yaml.h"

#include "networkTrackerYAML_utils.h"

using namespace std;

int main( int argc, char** argv )
{
//	YAML::Emitter out;
//	out << YAML::BeginMap;
//	out << YAML::Key << "AxisCamIP";
//	out << YAML::Value << "http://10.2.96.11/mjpg/video.mjpg";
//	out << YAML::Key << "cRIO_PI";
//	out << YAML::Value << "10.2.96.2";
//	out << YAML::Key << "cRIO_port";
//	out << YAML::Value << "1180";
//	out << YAML::Key << "Profile1";
//	out << YAML::Value	<< YAML::BeginMap
//						<< YAML::Key << "minH"
//						<< YAML::Value << 110
//						<< YAML::Key << "maxH"
//						<< YAML::Value << 140
//						<< YAML::Key << "noiseFilterSize"
//						<< YAML::Value << 5
//						<< YAML::Key << "smootherSize"
//						<< YAML::Value << 2
//						<< YAML::EndMap;
//	out << YAML::EndMap;
//	
//	cout << out.c_str() << endl;
	
	Parameters p = loadParameters();
	
	printf("cRIO_IP: %s\n", p.ipParams.cRIO_IP.c_str());
	
	printf("ActiveProfileIdx: %d\n", p.activeProfileIdx);
	
	printf("Profile0:\n\tminH: %d\n\tmaxH: %d\n", p.profiles[0].minH, p.profiles[0].maxH);
	
	printf("Profile3:\n\tminH: %d\n\tmaxH: %d\n", p.profiles[3].minH, p.profiles[3].maxH);
//	
//	p.maxH += 1;
//	
//	writeParameters(p);
//	
//	cout << p.maxH << endl;

}

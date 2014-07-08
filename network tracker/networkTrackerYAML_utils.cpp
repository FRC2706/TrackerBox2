#include "networkTrackerYAML_utils.h"

#include <unistd.h>

inline float clamp(float x, float a, float b){
    return x < a ? a : (x > b ? b : x);
}

inline int clamp(int x, int a, int b) {
    return x < a ? a : (x > b ? b : x);
}

void operator >> (const YAML::Node& node, IPParameters& p) {
	node["AxisCamAddress"] 	>> p.axisCamAddr;
	node["cRIO_IP"] 		>> p.cRIO_IP;
	node["cRIO_port"] 		>> p.cRIO_port;
}

void operator >> (const YAML::Node& node, ProfileParameters& p) {
    node["minH"] 			>> p.minH;
	node["maxH"] 			>> p.maxH;
	node["noiseFilterSize"] >> p.noiseFilterSize;
	node["smootherSize"] 	>> p.smootherSize;
	
	// clamp the values to valid ranges
	p.minH = clamp(p.minH, 0, 255);
	p.maxH = clamp(p.maxH, 0, 255);
	p.noiseFilterSize = clamp(p.noiseFilterSize, 0, 25);
	p.smootherSize = clamp(p.smootherSize, 0, 25);
}

void operator >> (const YAML::Node& node, ParticleReport& pr) {
	node["centerX"] >> pr.centerX;
	node["centerY"] >> pr.centerY;
	node["area"] >> pr.area;
	node["velX"] >> pr.velX;
	node["velY"] >> pr.velY;
}

bool operator == (Parameters& p1, Parameters& p2) {
	if (p1.ipParams != p2.ipParams)
		return false;
	
	if (p1.activeProfileIdx != p2.activeProfileIdx)
		return false;
	
	for(int i=0; i<10; i++)
		if (p1.profiles[i] != p2.profiles[i])
			return false;
	
	return true;
}

bool operator != (Parameters& p1, Parameters& p2) {
	return ! (p1 == p2);
}

bool operator == (IPParameters& ip1, IPParameters& ip2) {
	if (ip1.axisCamAddr.compare(ip2.axisCamAddr) != 0)
		return false;
	
	if (ip1.cRIO_IP.compare(ip2.cRIO_IP) != 0)
		return false;
		
	if (ip1.cRIO_port.compare(ip2.cRIO_port) != 0)
		return false;
	
	return true;
}

bool operator != (IPParameters& ip1, IPParameters& ip2) {
	return ! (ip1 == ip2);
}

bool operator == (ProfileParameters& pp1, ProfileParameters& pp2) {
	if (pp1.minH != pp2.minH || pp1.maxH != pp2.maxH)
		return false;
		
	if (pp1.noiseFilterSize != pp2.noiseFilterSize || pp1.smootherSize != pp2.smootherSize)
		return false;
	
	return true;
}

bool operator != (ProfileParameters& pp1, ProfileParameters& pp2) {
	return ! (pp1 == pp2);
}

Parameters loadParametersFromFile() {
	Parameters p;
	
	// check if the file exists on ramdisk, if not copy it from the sd card.
	// The first time we run the loop we'll need to copy it over.
	if( access( ramdiskParamsFile, F_OK ) == -1 ) {
		// file doesn't exist - so copy it over!
		std::ifstream  src(permanentParamsFile, std::ios::binary);
		std::ofstream  dst(ramdiskParamsFile,   std::ios::binary);

		dst << src.rdbuf();
		src.close();
		dst.close();
	}
	
	// Open the file from the ramdisk
	std::ifstream fin;
	while( !fin.is_open())
		fin.open(ramdiskParamsFile);
	YAML::Parser parser(fin);
    fin.close();
    
    // Use YAML tools to parse the file into c++ datastructures
    YAML::Node doc;
    parser.GetNextDocument(doc);
    
    doc >> p.ipParams;
    
    doc["ActiveProfileIdx"] >> p.activeProfileIdx;
    p.activeProfileIdx = clamp(p.activeProfileIdx, 0, 9);
    
    for (int i = 0; i < 10; i++) {
    	std::stringstream profileName;
		profileName << "Profile" << i;
    	
    	doc[profileName.str().c_str()] >> p.profiles[i];
    }
    
    return p;
}



void writeParametersToFile(Parameters p) {
	using namespace std;

	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "AxisCamAddress";
	out << YAML::Value << p.ipParams.axisCamAddr;
	out << YAML::Key << "cRIO_IP";
	out << YAML::Value << p.ipParams.cRIO_IP;
	out << YAML::Key << "cRIO_port";
	out << YAML::Value << p.ipParams.cRIO_port;
	out << YAML::Key << "ActiveProfileIdx";
	out << YAML::Value << p.activeProfileIdx;
	for(int i=0; i<10; i++) {
    	std::stringstream profileName;
		profileName << "Profile" << i;
		out << YAML::Key << profileName.str().c_str();
		out << YAML::Value << YAML::BeginMap
						<< YAML::Key << "minH"
						<< YAML::Value << p.profiles[i].minH
						<< YAML::Key << "maxH"
						<< YAML::Value << p.profiles[i].maxH
						<< YAML::Key << "noiseFilterSize"
						<< YAML::Value << p.profiles[i].noiseFilterSize
						<< YAML::Key << "smootherSize"
						<< YAML::Value << p.profiles[i].smootherSize
						<< YAML::EndMap;
	}
	
	out << YAML::EndMap;
	
	ofstream myfile;
	while(!myfile.is_open())
		myfile.open (ramdiskParamsFile);
	myfile << out.c_str();
	myfile.close();
	
	
	while(!myfile.is_open())
		myfile.open (permanentParamsFile);
	myfile << out.c_str();
	myfile.close();
}

ParticleReport loadParticleReportFromFile() {
	ParticleReport pr;
	
	std::ifstream fin;
	while (!fin.is_open())
		fin.open(ramdiskParamsFile);
    YAML::Parser parser(fin);
    fin.close();
    
    YAML::Node doc;
    parser.GetNextDocument(doc);
    doc >> pr;
    
    return pr;
}

void writeParticleReportToFile(ParticleReport pr) {
	using namespace std;

	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "centerX";
	out << YAML::Value << pr.centerX;
	out << YAML::Key << "centerY";
	out << YAML::Value << pr.centerY;
	out << YAML::Key << "area";
	out << YAML::Value << pr.area;
	out << YAML::Key << "velX";
	out << YAML::Value << pr.velX;
	out << YAML::Key << "velY";
	out << YAML::Value << pr.velY;
	out << YAML::EndMap;
	
	ofstream myfile;
	while(!myfile.is_open())
		myfile.open (ramdiskPartReportFile);
	myfile << out.c_str();
	myfile.close();
}

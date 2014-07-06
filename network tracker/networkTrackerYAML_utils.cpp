#include "networkTrackerYAML_utils.h"

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

Parameters loadParametersFromFile() {
	// read the parameters YAML file and update the global variables	
	Parameters p;
	
	std::ifstream fin;
	while( !fin.is_open())
		fin.open("parameters.yaml");
	YAML::Parser parser(fin);
    fin.close();
    
    YAML::Node doc;
    parser.GetNextDocument(doc);
    
    doc >> p.ipParams;
//	doc["AxisCamAddress"] 	>> p.ipParams.axisCamAddr;
//	doc["cRIO_IP"] 			>> p.ipParams.cRIO_IP;
//	doc["cRIO_port"] 		>> p.ipParams.cRIO_port;
    
    doc["ActiveProfileIdx"] >> p.activeProfileIdx;
    p.activeProfileIdx = clamp(p.activeProfileIdx, 0, 9);
    
    for (int i = 0; i < 10; i++) {
    	std::stringstream profileName;
		profileName << "Profile" << i;
    	
    	doc[profileName.str().c_str()] >> p.profiles[i];
//		doc[profileName.str().c_str()]["minH"] 			>> p.profiles[i].minH;
//		doc[profileName.str().c_str()]["maxH"] 			>> p.profiles[i].maxH;
//		doc[profileName.str().c_str()]["noiseFilterSize"] >> p.profiles[i].noiseFilterSize;
//		doc[profileName.str().c_str()]["smootherSize"] 	>> p.profiles[i].smootherSize;
	
		// clamp the values to valid ranges
		p.profiles[i].minH = clamp(p.profiles[i].minH, 0, 255);
		p.profiles[i].maxH = clamp(p.profiles[i].maxH, 0, 255);
		p.profiles[i].noiseFilterSize = clamp(p.profiles[i].noiseFilterSize, 0, 25);
		p.profiles[i].smootherSize = clamp(p.profiles[i].smootherSize, 0, 25);
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
		myfile.open ("parameters.yaml");
	myfile << out.c_str();
	myfile.close();
}

ParticleReport loadParticleReportFromFile() {
	ParticleReport pr;
	
	std::ifstream fin;
	while (!fin.is_open())
		fin.open("particleReport.yaml");
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
		myfile.open ("particleReport.yaml");
	myfile << out.c_str();
	myfile.close();
}

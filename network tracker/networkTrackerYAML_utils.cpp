#include "networkTrackerYAML_utils.h"

#include <unistd.h>
#include <fstream>
#include <iostream>

inline float clamp(float x, float a, float b){
    return x < a ? a : (x > b ? b : x);
}

inline int clamp(int x, int a, int b) {
    return x < a ? a : (x > b ? b : x);
}

// be able to use the equals operator == between two Parameters
bool operator == (Parameters& p1, Parameters& p2) {
	if (p1.ipCamAddr != p2.ipCamAddr)
		return false;

	if (p1.minHue != p2.minHue || p1.maxHue != p2.maxHue)
		return false;

	if (p1.minSat != p2.minSat || p1.maxSat != p2.maxSat)
		return false;

	if (p1.minVal != p2.minVal || p1.maxVal != p2.maxVal)
		return false;

	if (p1.erodeDilateSize != p2.erodeDilateSize)
		return false;

	if (p1.minTargetArea != p2.minTargetArea)
		return false;

	return true;
}

// be able to use the not-equals operator != between two Parameters
bool operator != (Parameters& p1, Parameters& p2) {
	return ! (p1 == p2);
}


Parameters loadParametersFromFile() {
    YAML::Node doc = YAML::LoadFile(permanentParamsFile);

	Parameters p;

    // load the IP address of the ip camera
    p.ipCamAddr = doc["ipCamAddr"].as<std::string>();

	// load the HSV params
	p.minHue = clamp(doc["minHue"].as<int>(), 0, 255);
	p.maxHue = clamp(doc["maxHue"].as<int>(), 0, 255);
	p.minSat = clamp(doc["minSat"].as<int>(), 0, 255);
	p.maxSat = clamp(doc["maxSat"].as<int>(), 0, 255);
	p.minVal = clamp(doc["minVal"].as<int>(), 0, 255);
	p.maxVal = clamp(doc["maxVal"].as<int>(), 0, 255);

	// load the vision params
	p.erodeDilateSize = doc["erodeDilateSize"].as<int>();

	p.minTargetArea = doc["minTargetArea"].as<int>();

    return p;
}



void writeParametersToFile(Parameters p) {
	using namespace std;

	YAML::Node out;

	// save the IP address of the ip camera
	out["ipCamAddr"] = p.ipCamAddr;

	// save the HSV params
	out["minHue"] = p.minHue;
	out["maxHue"] = p.maxHue;
	out["minSat"] = p.minSat;
	out["maxSat"] = p.maxSat;
	out["minVal"] = p.minVal;
	out["maxVal"] = p.maxVal;

	// save the vision params
	out["erodeDilateSize"] = p.erodeDilateSize;

	out["minTargetArea"] = p.minTargetArea;


	ofstream myfile;
	myfile.open(permanentParamsFile);
	if(! myfile.is_open() )
	cerr << "Error: Could not open the file " << permanentParamsFile << " for writing." << endl;
	else {
		myfile << out;
		myfile.close();
	}
}


// TODO: once we define what the vision report will be, we can fill this out
void writeVisionReportToFile(VisionReport vr) {
	using namespace std;

	// YAML::Emitter out;
	// out << YAML::BeginMap;
	// out << YAML::Key << "centerX";
	// out << YAML::Value << pr.centerX;
	// out << YAML::Key << "centerY";
	// out << YAML::Value << pr.centerY;
	// out << YAML::Key << "area";
	// out << YAML::Value << pr.area;
	// out << YAML::Key << "velX";
	// out << YAML::Value << pr.velX;
	// out << YAML::Key << "velY";
	// out << YAML::Value << pr.velY;
	// out << YAML::EndMap;

  // YAML::Node out;
  // ut["centerX"] = pr.centerX;
  // ut["centerY"] = pr.centerY;
  // ut["area"] = pr.area;
  // ut["velX"] = pr.velX;
  // ut["velY"] =  pr.velY;
  //
  // fstream myfile;
  // myfile.open(ramdiskPartReportFile);
  // f(! myfile.is_open() )
  //   cerr << "Error: Could not open the file " << ramdiskPartReportFile << " for writing." << endl;
  // else {
  // 	myfile << out;
  // 	myfile.close();
  // }
}

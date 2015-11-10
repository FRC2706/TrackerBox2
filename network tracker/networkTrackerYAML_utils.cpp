#include "networkTrackerYAML_utils.h"

#include <unistd.h>

inline float clamp(float x, float a, float b){
    return x < a ? a : (x > b ? b : x);
}

inline int clamp(int x, int a, int b) {
    return x < a ? a : (x > b ? b : x);
}

// be able to use the >> operator to load a YAML object into an IPParameters
void operator >> (const YAML::Node& node, IPParameters& p) {
  p.axisCamAddr = node["AxisCamAddress"].as<std::string>();
  p.cRIO_IP = node["cRIO_IP"].as<std::string>();
  p.cRIO_port = node["cRIO_port"].as<std::string>();
}

// be able to use the >> operator to load a YAML object into a ProfileParameters
void operator >> (const YAML::Node& node, ProfileParameters& p) {
  p.minH = node["minH"].as<int>();
  p.maxH = node["maxH"].as<int>();
	p.noiseFilterSize = node["noiseFilterSize"].as<int>();;
  p.smootherSize = node["smootherSize"].as<int>();;

	// clamp the values to valid ranges
	p.minH = clamp(p.minH, 0, 255);
	p.maxH = clamp(p.maxH, 0, 255);
	p.noiseFilterSize = clamp(p.noiseFilterSize, 0, 25);
	p.smootherSize = clamp(p.smootherSize, 0, 25);
}

// be able to use the >> operator to load a YAML object into a ParticleReport
void operator >> (const YAML::Node& node, ParticleReport& pr) {
	pr.centerX = node["centerX"].as<double>();
	pr.centerY = node["centerY"].as<double>();
	pr.area = node["area"].as<double>();
	pr.velX = node["velX"].as<double>();
	pr.velY = node["velY"].as<double>();
}

// be able to use the equals operator == between two Parameters
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

// be able to use the not-equals operator != between two Parameters
bool operator != (Parameters& p1, Parameters& p2) {
	return ! (p1 == p2);
}

// be able to use the equals operator == between two IPParameters
bool operator == (IPParameters& ip1, IPParameters& ip2) {
	if (ip1.axisCamAddr.compare(ip2.axisCamAddr) != 0)
		return false;

	if (ip1.cRIO_IP.compare(ip2.cRIO_IP) != 0)
		return false;

	if (ip1.cRIO_port.compare(ip2.cRIO_port) != 0)
		return false;

	return true;
}

// be able to use the not-equals operator != between two IPParameters
bool operator != (IPParameters& ip1, IPParameters& ip2) {
	return ! (ip1 == ip2);
}

// be able to use the equals operator == between two ProfileParameters
bool operator == (ProfileParameters& pp1, ProfileParameters& pp2) {
	if (pp1.minH != pp2.minH || pp1.maxH != pp2.maxH)
		return false;

	if (pp1.noiseFilterSize != pp2.noiseFilterSize || pp1.smootherSize != pp2.smootherSize)
		return false;

	return true;
}

// be able to use the not-equals operator != between two ProfileParameters
bool operator != (ProfileParameters& pp1, ProfileParameters& pp2) {
	return ! (pp1 == pp2);
}

Parameters loadParametersFromFile() {
	Parameters p;

	// check if the parameters file exists on ramdisk, if not copy it from the sd card.
	// The first time we run the loop we'll need to copy it over.
	if( access( ramdiskParamsFile, F_OK ) == -1 ) {
		// file doesn't exist - so copy it over!
		std::ifstream  src(permanentParamsFile, std::ios::binary);
		std::ofstream  dst(ramdiskParamsFile,   std::ios::binary);

		dst << src.rdbuf();
		src.close();
		dst.close();
	}

    YAML::Node doc = YAML::LoadFile(ramdiskParamsFile);

    // load the IP addresses
    doc >> p.ipParams;

    p.activeProfileIdx = doc["ActiveProfileIdx"].as<int>();
    p.activeProfileIdx = clamp(p.activeProfileIdx, 0, 9);

    // grab each of the profiles
    for (int i = 0; i < 10; i++) {
    	std::stringstream profileName;
      profileName << "Profile" << i;

    	doc[profileName.str().c_str()] >> p.profiles[i];
    }

    return p;
}



void writeParametersToFile(Parameters p) {
	using namespace std;

  YAML::Node out;
  out["AxisCamAddress"] = p.ipParams.axisCamAddr;
  out["cRIO_IP"] = p.ipParams.cRIO_IP;
  out["cRIO_port"] = p.ipParams.cRIO_port;
  out["ActiveProfileIdx"] = p.activeProfileIdx;

  for(int i=0; i<10; i++) {
    YAML::Node profile;
    profile["minH"] = p.profiles[i].minH;
    profile["maxH"] = p.profiles[i].maxH;
    profile["noiseFilterSize"] = p.profiles[i].noiseFilterSize;
    profile["smootherSize"] = p.profiles[i].smootherSize;

    std::stringstream profileName;
    profileName << "Profile" << i;
    out[profileName.str().c_str()] = profile;
	}

	ofstream myfile;
  myfile.open(ramdiskParamsFile);
  if(! myfile.is_open() )
    cerr << "Error: Could not open the file " << ramdiskParamsFile << " for writing." << endl;
  else {
  	myfile << out;
  	myfile.close();
  }

  myfile.open(permanentParamsFile);
	if(! myfile.is_open() )
    cerr << "Error: Could not open the file " << ramdiskParamsFile << " for writing." << endl;
  else {
  	myfile << out;
  	myfile.close();
  }
}


ParticleReport loadParticleReportFromFile() {
	ParticleReport pr;

  YAML::Node doc = YAML::LoadFile(ramdiskPartReportFile);
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

# 1.
Cxx=g++
# 2.
CXXFLAGS = -Wall `pkg-config --cflags opencv yaml-cpp`

VALGRIND_FLAGS = -g -O0
# 3.
OpenCVLibs = `pkg-config --libs opencv`
YAMLLibs = `pkg-config --libs yaml-cpp`

networkTracker_withServer: networkTracker_withServer.cpp networkTracker.h networkTrackerYAML_utils.o
	$(Cxx) $(CXXFLAGS) -O3 $^ -o $@ $(OpenCVLibs) $(YAMLLibs)
	
networkTracker: networkTracker.cpp networkTracker.h networkTrackerYAML_utils.o
	$(Cxx) $(CXXFLAGS) -O3 $^ -o $@ $(OpenCVLibs) $(YAMLLibs)
	
	
generateYAML: generateYAML.cpp networkTrackerYAML_utils.o
	$(Cxx) $(CXXFLAGS) -O3 $^ -o $@ $(YAMLLibs)
	
networkTrackerYAML_utils.o: networkTrackerYAML_utils.h networkTrackerYAML_utils.cpp
	$(Cxx) $(CXXFLAGS) -O3 -c $^  $(YAMLLibs)
	
socketServer_changeProfile: socketServer_changeProfile.cpp networkTrackerYAML_utils.o
	gcc $(CXXFLAGS) -O3 $^ -o $@ $(YAMLLibs)
	
socketServer_dataRequest: socketServer_dataRequest.cpp networkTrackerYAML_utils.o
	gcc $(CXXFLAGS) -O3 $^ -o $@  $(YAMLLibs)
	
all: networkTracker socketServer_changeProfile socketServer_dataRequest

clean: 
	rm -f *~ *.o networkTracker generateYAML socketServer_changeProfile socketServer_dataRequest

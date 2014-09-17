# 1.
Cxx=g++
# 2.
CXXFLAGS = -Wall `pkg-config --cflags opencv yaml-cpp`

VALGRIND_FLAGS = -g -O0
# 3.
OpenCVLibs = `pkg-config --libs opencv`
YAMLLibs = `pkg-config --libs yaml-cpp`
	
networkTracker: networkTracker.cpp networkTracker.h networkTrackerYAML_utils.o
	$(Cxx) $(CXXFLAGS) -O2 $^ -o $@ $(OpenCVLibs) $(YAMLLibs) -lpthread -lzmq
	
networkTrackerYAML_utils.o: networkTrackerYAML_utils.h networkTrackerYAML_utils.cpp
	$(Cxx) $(CXXFLAGS) -O2 -c $^  $(YAMLLibs)
	
all: networkTracker

clean: 
	rm -f *~ *.o networkTracker generateYAML socketServer_changeProfile socketServer_dataRequest

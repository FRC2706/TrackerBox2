# 1. which compiler to use
Cxx=g++

# 2. compiler flags
CXXFLAGS = -Wall -O2 `pkg-config --cflags opencv yaml-cpp`
VALGRIND_FLAGS = -Wall -g -O0 `pkg-config --cflags opencv yaml-cpp`

# 3. libraries to link against
OpenCVLibs = `pkg-config --libs opencv`
YAMLLibs = `pkg-config --libs yaml-cpp`
OTHERLibs = -lpthread

# 4. name of the output executable
OUTPUT_EXE = trackerbox2

# 5. rules

all: $(OUTPUT_EXE)

$(OUTPUT_EXE): networkTracker.cpp networkTracker.h networkTrackerYAML_utils.o vision_utils.o
	$(Cxx) $(CXXFLAGS) $(filter-out %.h, $^) -o $@ $(OpenCVLibs) $(YAMLLibs) $(OTHERLibs)

networkTrackerYAML_utils.o: networkTrackerYAML_utils.h networkTrackerYAML_utils.cpp
	$(Cxx) $(CXXFLAGS) -c $(filter-out %.h, $^) $(YAMLLibs)

vision_utils.o: vision_utils.h vision_utils.cpp
		$(Cxx) $(CXXFLAGS) -c $(filter-out %.h, $^) $(YAMLLibs)

clean:
	rm -f *~ *.o $(OUTPUT_EXE)

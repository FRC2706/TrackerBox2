# 1. which compiler to use
Cxx=g++

# 2. compiler flags
CXXFLAGS = -Wall -O2 `pkg-config --cflags opencv`

# 3. libraries to link against
OpenCVLibs = `pkg-config --libs opencv`
# 4. name of the output executable

# 5. rules

all: openCVcontours findHeights

openCVcontours: OpenCVcontours.c
	$(Cxx) $(CXXFLAGS) $(filter-out %.h, $^) -o $@ $(OpenCVLibs)

findHeights: FindHeights.c
	$(Cxx) $(CXXFLAGS) $(filter-out %.h, $^) -o $@ $(OpenCVLibs)

clean:
	rm -f *~ *.o $(OUTPUT_EXE)

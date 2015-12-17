# 1. which compiler to use
Cxx=g++

# 2. compiler flags
CXXFLAGS = -Wall -O2 `pkg-config --cflags opencv yaml-cpp`
VALGRIND_FLAGS = -Wall -g -O0 `pkg-config --cflags opencv`

# 3. libraries to link against
OpenCVLibs = `pkg-config --libs opencv`

# 4. name of the output executable
OUTPUT_EXE = benchmarkOpenCV

# 5. rules

all: $(OUTPUT_EXE)

$(OUTPUT_EXE): benchmarkOpenCV.cpp
	$(Cxx) $(CXXFLAGS) $(filter-out %.h, $^) -o $@ $(OpenCVLibs)

clean:
	rm -f *~ *.o $(OUTPUT_EXE)

## Computer vision benchmarking tool
This is a tool to measure how many frames-per-second we can get doing typical computer vision processing.

### Dependencies
This uses the linux system time calls for timing. I don't expect that it would work on Windows.

Building this tool requires that you have **opencv-dev** installed. On a debian-based linux distro (Ubuntu, Raspbian, etc) you can install it by doing:

    sudo apt-get install libopencv-dev

### Building

`cd` into the folder `benchmarking tool` and run

    make

To remove the executable, run

    make clean


### Running
To run the tool you need to provide it an image file to process. I have provided several sample images at different resolutions for you to run it on.

    ./benchmarkOpenCV 640x480.jpg

### Viewing the processed image

Just for fun, if you want to see the output of the edge detection, open the file `benchmarkOpenCV.cpp` and change the 0 to a 1 in the line

    13 #define SHOW_GUI 0

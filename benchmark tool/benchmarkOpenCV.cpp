#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <inttypes.h>
#include <time.h>
#include <signal.h>

#include <opencv2/core/core.hpp>
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <iostream>

#define SHOW_GUI 0


using namespace cv;

int framesProcessed = 0;
struct timespec tstart={0,0}, tend={0,0};
Mat image, src_gray, dst, detected_edges;


int edgeThresh = 1;
int lowThreshold = 30;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;


void printStats() {
  clock_gettime(CLOCK_MONOTONIC, &tend);
  double seconds =
         ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
         ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);

   double fps = framesProcessed / seconds;

  printf("Ran for %.2f s, at a frame-rate of %.2f fps\n", seconds, fps);
}

/**
 * This catches a ctrl-c (also called an interupt signal, or sigint),
 * it prints the stats so far and quits.
 */
void sigintHandler(int dummy) {
    printStats();
    exit(0);
}

/**
 * As an example of a typical computer vision task, we'll do edge detection.
 */
void doOpenCVStuff() {
  /// Convert the image to grayscale
  cvtColor( image, src_gray, CV_BGR2GRAY );

  /// Reduce noise with a kernel 3x3
  // blur( src_gray, detected_edges, Size(7,7) );
  GaussianBlur( src_gray, detected_edges, Size(11,11), 1.7, 1.7);

  /// Canny detector
  Canny( detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size );

  #if SHOW_GUI
    // just for fun, let's look at it!
    namedWindow( "Display window", WINDOW_AUTOSIZE );// Create a window for display.
    imshow( "Display window", detected_edges );
    waitKey(10);
  #endif
}


int main( int argc, char** argv )
{
    // register a ctrl-c handler
    signal(SIGINT, sigintHandler);

    // load the image from disk
    if( argc != 2) {
        printf("Please give me an image file!\n\nUsage: ./benchmarkOpenCV <ImageFile>\n");
        return -1;
    }

    image = imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

    if(! image.data ) {                             // Check for invalid input
      printf("Could not open or find the image\n");
      return -1;
    }

    // Create a matrix of the same type and size as src (for dst)
    dst.create( image.size(), image.type() );

    printf("Running the benchmark for 10 s ...\n");

    // start the timer - anything after this will be measured.
    clock_gettime(CLOCK_MONOTONIC, &tstart);

    while(1) {
    clock_gettime(CLOCK_MONOTONIC, &tend);
    double seconds =
           ((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) -
           ((double)tstart.tv_sec + 1.0e-9*tstart.tv_nsec);
    if (seconds >= 10)
      break;

      // do one frame of work
      doOpenCVStuff();

      framesProcessed++;
    }

    printStats();
}

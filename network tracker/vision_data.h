#ifndef VISION_DATA__H
#define VISION_DATA__H

struct VisionTarget {
 float boundingArea;     //% of cam [0, 10]

  //center of target

  float ctrX;             //[-1.0,1.0]
  float ctrY;             //[-1.0,1.0]

  float aspectRatio;
};

struct VisionReport {

  // The number of targets that were detected.
  int numTargetsFound;

  // An array of vision targets.
  VisionTarget* targetsFound;
};

#endif

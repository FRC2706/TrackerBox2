#ifndef VISION_DATA__H
#define VISION_DATA__H

#define DATA_VERSION_CODE Stronghold2017

struct VisionTarget {

  // the area of the bounding box of this target, as a fraction of the total area of the image.
  float boundingArea;     // % of cam [0, 1.0]

  //center of target
  float ctrX;             // [-1.0, 1.0]
  float ctrY;             // [-1.0, 1.0]
};

struct VisionReport {

  // The number of targets that were detected.
  int numTargetsFound;

  // An array of vision targets.
  VisionTarget* targetsFound;
};

#endif

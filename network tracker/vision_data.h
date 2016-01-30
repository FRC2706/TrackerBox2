#ifndef VISION_DATA__H
#define VISION_DATA__H

struct VisionTarget {
  float centreX;
  float centreY;

  float area;

  float leftHeight;
  float rightHeight;
};

struct VisionReport {

  // The number of targets that were detected.
  int numTargetsFound;

  // An array of vision targets.
  VisionTarget* targetsFound;
};

#endif

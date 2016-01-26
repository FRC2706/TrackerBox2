#ifndef VISION_DATA__H
#define VISION_DATA__H

struct VisionTarget {

  // The location of the centre of the vision target relative to the centre of
  // camera. These are floats between [-1.0, 1.0] where -1.0 is the left / top of the image,
  // and +1.0 is the right / bottom of the image;
  float centreX;
  float centreY;

  // This is a float in [0, 1.0] representing the fraction of the image area
  // occupied by the vision target's bounding box. ie an area of 1.0 means that
  // the vision target perfectly fills the image, an area of 0.1 means that the
  // vision target fills 10% of the image.
  float area;

  // These are floats in [0, 1.0] representing the heights of the left and right
  // vertical bars of the vision target as a fraction of the image height.
  // Comparing these numbers shows how skewed, or off-centre we are from the target.
  float leftHeight;
  float rightHeight;
};

struct VisionReport {

  // The number of targets that were detected.
  int numTargetsFound;

  // An array of vision targets.
  VisionTarget* targetsFound;
}

#endif

#ifndef RAYCAST_H
#define RAYCAST_H

#include "sector.h"
#include "utils.h"

// The result of a raycast:
// hit represents if the raycast was successful.
// If the hit was unsuccessful, none of the other variables are initalised.
// pos is the position where the ray hit
// distance is how far the point was from where the ray started
// line_id is the ID of the line that was hit.
typedef struct {
  int hit;
  vec2f pos;
  float distance;
  int line_id;
  int sector_id;
} Raycast;

Raycast raycast_sec(Sector *sector, vec2f pos, float rot, float distance);
Raycast raycast(vec2f pos, float rot, float distance);

#endif

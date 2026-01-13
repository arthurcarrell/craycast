#include "utils.h"
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

vec2f get_line_intersections(const Line *a, const Line *b, int *found) {
  float dx1 = a->end.x - a->start.x;
  float dy1 = a->end.y - a->start.y;
  float dx2 = b->end.x - b->start.x;
  float dy2 = b->end.y - b->start.y;

  float denom = dx1 * dy2 - dy1 * dx2;

  if (fabsf(denom) < FLT_EPSILON) {
    *found = 0;
    return (vec2f){0, 0};
  }

  float dx = b->start.x - a->start.x;
  float dy = b->start.y - a->start.y;
  float s = (dx * dy2 - dy * dx2) / denom;
  float t = (dx * dy1 - dy * dx1) / denom;

  if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
    *found = 1;
    return (vec2f){a->start.x + s * dx1, a->start.y + s * dy1};
  } else {
    *found = 0;
    return (vec2f){0, 0};
  }
}

float get_distance(vec2f start, vec2f end) {
  float dx = end.x - start.x;
  float dy = end.y - start.y;
  return sqrtf(dx * dx + dy * dy); // pythagorus you are the goat
}

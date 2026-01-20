#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>

// Helper macro
#define NINETY_DEGINRAD 1.570796

typedef struct {
  int x;
  int y;
} vec2i;

typedef struct {
  float x;
  float y;
} vec2f;

typedef struct {
  int r;
  int g;
  int b;
  int a;
} rgba;

// --- Direction and Distance ---

// Returns the distance between two points
float get_distance(vec2f pos1, vec2f pos2);
// Returns a point translated using a rotation and the amount to move by
vec2f add_direction(vec2f pos, float rot, float amount);
// Returns the direction that 'start' would need to face to look at 'end'
float get_direction(vec2f start, vec2f end);
// Converts degrees to radians
float deg_to_radians(float degrees);

// --- Clamp functions ---

// returns a float that cannot exceed max and cannot go below min
float clampf(float num, float max, float min);
// returns an int that cannot exceed max and cannot go below min
int clamp(int num, int max, int min);

// --- Misc ---
inline vec2i asvec2i(vec2f);
int rgba_to_int(rgba color);

#endif // !UTILS_H

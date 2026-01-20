#include "utils.h"
#include "framebuf.h"
#include <SDL3/SDL_pixels.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

float get_distance(vec2f start, vec2f end) {
  float dx = end.x - start.x;
  float dy = end.y - start.y;
  return sqrtf(dx * dx + dy * dy); // pythagorus you are the goat
}

vec2f add_direction(vec2f pos, float rot, float amount) {
  return (vec2f){.x = pos.x + amount * cosf(rot),
                 .y = pos.y + amount * sinf(rot)};
}

float get_direction(vec2f start, vec2f end) {
  return atan2f(end.y - start.y, end.x - start.x);
}

float deg_to_radians(float degrees) { return degrees * (M_PI / 180); }

// returns a float that cannot exceed max and cannot go below min
float clampf(float num, float max, float min) {
  if (num > max) {
    return max;
  }
  if (num < min) {
    return min;
  }

  return num;
}

int clamp(int num, int max, int min) {
  if (num > max) {
    return max;
  }
  if (num < min) {
    return min;
  }

  return num;
}

inline vec2i asvec2i(vec2f vec2) { return (vec2i){vec2.x, vec2.y}; }

int rgba_to_int(rgba color) {
  return SDL_MapRGBA(framebuf.format, NULL, color.r, color.g, color.b, color.a);
}

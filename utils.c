#include "utils.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

vec2f get_line_intersections(const Line *a, const Line *b, int *found) {
  // https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection
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

vec2f add_direction(vec2f pos, float rot, float amount) {
  return (vec2f){.x = pos.x + amount * cosf(rot),
                 .y = pos.y + amount * sinf(rot)};
}

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

// Lines
Line create_simple_line(vec2f start, vec2f end) {
  return (Line){.start = start, .end = end, .flags = 0};
}

Line create_line_with_flags(vec2f start, vec2f end, unsigned int flags) {
  return (Line){.start = start, .end = end, .flags = flags};
}

Line create_render_line(vec2f start, vec2f end, rgba color) {
  return (Line){.start = start, .end = end, .color = color};
}

Line create_portal_line(vec2f start, vec2f end, int output_id,
                        float output_rot) {
  return (Line){.start = start,
                .end = end,
                .portal = {.output_id = output_id, .output_rot = output_rot},
                .flags = LINE_FLAG_PORTAL};
}

int is_on_line(vec2f pos, Line line, float precision) {
  // https://stackoverflow.com/questions/11907947/how-to-check-if-a-point-lies-on-a-line-between-2-other-points
  float dxc = pos.x - line.start.x;
  float dyc = pos.y - line.start.y;
  float dxl = line.end.x - line.start.x;
  float dyl = line.end.y - line.start.y;

  float cross = dxc * dyl - dyc * dxl;

  if (fabsf(cross) > precision) {
    return 0;
  } else {
    return 1;
  }
}

float get_line_percent(vec2f pos, Line line) {
  float dxc = pos.x - line.start.x;
  float dyc = pos.y - line.start.y;
  float dxl = line.end.x - line.start.x;
  float dyl = line.end.y - line.start.y;

  float length_sqr = dxl * dxl + dyl * dyl;

  if (length_sqr < FLT_EPSILON) {
    return 0;
  }

  return (dxc * dxl + dyc * dyl) / length_sqr;
}

#include "line.h"
#include "utils.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

vec2f get_line_intersections(Line *a, Line *b, int *found) {
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

// Line helper functions - unused at the moment
Line create_simple_line(vec2f start, vec2f end) {
  return (Line){.start = start, .end = end};
}

LineSegment create_line_with_flags(vec2f start, vec2f end, unsigned int flags) {
  return (LineSegment){.start = start, .end = end, .flags = flags};
}

Line create_render_line(vec2f start, vec2f end, rgba color) {
  return (Line){.start = start, .end = end, .color = color};
}

LineSegment create_portal_line(vec2f start, vec2f end, int output_id,
                               int flipped) {
  Portal *portal = malloc(sizeof(Portal));
  if (portal == NULL) {
    exit(1);
  }
  *portal = (Portal){output_id, flipped};
  return (LineSegment){
      .start = start, .end = end, .portal = portal, .flags = LINE_FLAG_PORTAL};
}

int is_on_line(vec2f pos, Line line, float tolerance) {
  // https://stackoverflow.com/questions/11907947/how-to-check-if-a-point-lies-on-a-line-between-2-other-points
  float dxc = pos.x - line.start.x;
  float dyc = pos.y - line.start.y;
  float dxl = line.end.x - line.start.x;
  float dyl = line.end.y - line.start.y;

  float cross = dxc * dyl - dyc * dxl;
  float dot = dxc * dxl + dyc * dyl;

  if (dot < 0) {
    return 0;
  }

  if (dot > pow(get_distance(line.start, line.end), 2)) {
    return 0;
  }

  if (fabsf(cross) > tolerance) {
    return 0;
  } else {
    return 1;
  }
}

LineSegment *get_line_at_point(vec2f point, LineSegment *lines, int count,
                               float tolerance) {
  for (int i = 0; i < count; i++) {
    if (is_on_line(point, lineseg_line(lines[i]), tolerance)) {
      return &lines[i];
    }
  }

  return NULL;
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

Line lineseg_line(LineSegment line) {
  return (Line){line.start, line.end, line.color};
}

void destroy_linesegs(LineSegment *lines, int *line_count) {
  int count = 0;
  int portal_count = 0;
  for (int i = 0; i < *line_count; i++) {
    if (lines[i].flags & LINE_FLAG_PORTAL ||
        lines[i].flags & LINE_FLAG_PORTAL_EXIT) {
      // free the portal
      free(lines[i].portal);
      lines[i].portal = NULL;
      portal_count++;
    }
    count++;
  }

  free(lines);
  *line_count = 0;

  printf("Destroyed %d line segments and destroyed %d portals\n", count,
         portal_count);
}

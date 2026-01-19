#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>

// Line flags
#define LINE_FLAG_PORTAL (1u << 1)
#define LINE_FLAG_PORTAL_EXIT (1u << 2)

// Helper macro
#define NINETY_DEGINRAD 1.570796

typedef struct {
  int x;
  int y;
} vec2;

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

// In its most simple form, a struct that has a start and an end point
// Gets pretty complicated from there on out though - may be refactored
typedef struct {
  vec2f start;
  vec2f end;
  rgba color;
  int id;
  unsigned int flags;
  struct {
    int output_id;
    int flipped;
  } portal;
} Line;

// --- Line Intersections (the whole project runs on this) ---

// Checks if two lines touch each other at any point.
// Found is 1 if the lines touch and is 0 if they dont.
// The value returned is the point at which the two lines touch.
// If they dont, the point is returned as 0,0 - but found should be used to
// check if they do.
vec2f get_line_intersections(const Line *line1, const Line *line2, int *found);

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

//  --- Line functions ---
Line create_simple_line(vec2f start, vec2f end);
Line create_line_with_flags(vec2f start, vec2f end, unsigned int flags);
Line create_render_line(vec2f start, vec2f end, rgba color);
Line create_portal_line(vec2f start, vec2f end, int output_id, int flipped);

// Checks if a point is on a line. Returns 0 for no and 1 for yes.
// tolerance is how far the point can be from the line for it to still count
// as being on the line. For most uses, tolerance should be 0 but for uses like
// checking if the mouse is touching the line, tolerance should be high to allow
// easier interaction.
int is_on_line(vec2f pos, Line line, float tolerance);

// Returns a decimal percentage on how far a position is on the line.
// 0 is the start and 1 is the end.
float get_line_percent(vec2f pos, Line line);
#endif // !UTILS_H

#ifndef LINE_H
#define LINE_H

#include "utils.h"

// Line flags
#define LINE_FLAG_PORTAL (1u << 1)
#define LINE_FLAG_PORTAL_EXIT (1u << 2)

// In its most simple form, a struct that has a start and an end point
// Gets pretty complicated from there on out though - may be refactored
typedef struct {
  vec2f start;
  vec2f end;
  rgba color;
} Line;

typedef struct {
  int output_id;
  int flipped;
} Portal;

typedef struct {
  vec2f start;
  vec2f end;
  rgba color;
  int id;
  int sector_id;
  unsigned int flags;
  Portal *portal;
} LineSegment;

// --- Line Intersections (the whole project runs on this) ---

// Checks if two lines touch each other at any point.
// Found is 1 if the lines touch and is 0 if they dont.
// The value returned is the point at which the two lines touch.
// If they dont, the point is returned as 0,0 - but found should be used to
// check if they do.
vec2f get_line_intersections(Line *line1, Line *line2, int *found);

//  --- Line functions ---
Line create_simple_line(vec2f start, vec2f end);
LineSegment create_line_with_flags(vec2f start, vec2f end, unsigned int flags);
Line create_render_line(vec2f start, vec2f end, rgba color);
LineSegment create_portal_line(vec2f start, vec2f end, int output_id,
                               int flipped);

Line lineseg_line(LineSegment line);
// Checks if a point is on a line. Returns 0 for no and 1 for yes.
// tolerance is how far the point can be from the line for it to still count
// as being on the line. For most uses, tolerance should be 0 but for uses like
// checking if the mouse is touching the line, tolerance should be high to allow
// easier interaction.
int is_on_line(vec2f pos, Line line, float tolerance);

// Returns a decimal percentage on how far a position is on the line.
// 0 is the start and 1 is the end.
float get_line_percent(vec2f pos, Line line);

// Destroys all line segments and sets line count to 0.
void destroy_linesegs(LineSegment *lines, int *line_count);

#endif

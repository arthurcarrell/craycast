#ifndef SECTOR_H
#define SECTOR_H

#include "line.h"
#include "utils.h"

typedef struct {
  // structure ID, used for lookups
  int id;

  // line data
  LineSegment *lines;
  int line_count;

  // height
  int floor_height;
  int ceil_height;

  // color
  rgba floor_color;
  rgba ceil_color;

  int light_modifer;
} Sector;

void destroy_sectors(Sector **sectors, int *amount);
void destroy_sector(Sector *sector);
Sector *sector_create(int max_lines, rgba ceil_color, int ceil_height,
                      rgba floor_color, int floor_height, int light_modifer);

// Checks if a vec2f is in a sector and returns the result.
int is_point_in_sector(vec2f point, Sector *sector);

// Returns the sector that the point is in. Points can only be in one sector at
// a time. Returns NULL if the point is not in a sector
Sector *get_sector_of_point(vec2f point);

#endif

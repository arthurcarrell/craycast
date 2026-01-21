#include "sector.h"
#include "line.h"
#include "state.h"
#include "utils.h"
#include <math.h>
#include <stdlib.h>

void destroy_sector(Sector *sector) {
  destroy_linesegs(sector->lines, &sector->line_count);
}

void destroy_sectors(Sector **sectors, int *amount) {
  for (int i = 0; i < *amount; i++) {
    destroy_sector(&(*sectors)[i]);
  }
  free(*sectors);
  *sectors = NULL;
  *amount = 0;
}

Sector *sector_create(int max_lines, rgba ceil_color, int ceil_height,
                      rgba floor_color, int floor_height, int light_modifer) {
  LineSegment *lines = calloc(10, sizeof(LineSegment));
  Sector sector = (Sector){.id = state.sector_count,
                           .line_count = 0,
                           .lines = lines,
                           .ceil_color = ceil_color,
                           .ceil_height = ceil_height,
                           .floor_color = floor_color,
                           .floor_height = floor_height,
                           .light_modifer = light_modifer};

  state.sectors[state.sector_count] = sector;
  state.sector_count++;

  return &state.sectors[state.sector_count - 1];
}

int is_point_in_sector(vec2f point, Sector *sector) {
  // https://www.geeksforgeeks.org/dsa/how-to-check-if-a-given-point-lies-inside-a-polygon/
  // - Adapted for C
  int line_count = sector->line_count;
  LineSegment *linesegs = sector->lines;
  int x = point.x;
  int y = point.y;
  int inside = 0;

  for (int i = 0; i < sector->line_count; i++) {

    LineSegment *seg = &sector->lines[i];
    vec2f p1 = seg->start;
    vec2f p2 = seg->end;

    // Ignore horizontal edges
    if (fabsf(p1.y - p2.y) < 1e-6f)
      continue;

    // Check if point y is within vertical range of edge
    if (y > fminf(p1.y, p2.y) && y <= fmaxf(p1.y, p2.y) &&
        x <= fmaxf(p1.x, p2.x)) {

      float xintersect = (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;

      if (p1.x == p2.x || x <= xintersect) {
        inside = !inside;
      }
    }
  }

  return inside;
}

Sector *get_sector_of_point(vec2f point) {
  for (int i = 0; i < state.sector_count; i++) {
    Sector *sector = &state.sectors[i];
    if (is_point_in_sector(point, sector)) {
      return sector;
    }
  }

  return NULL;
}

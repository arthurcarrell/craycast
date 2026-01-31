#include "../sector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_line(FILE *fptr, LineSegment *lineseg) {
  fprintf(fptr, "lineseg %d %d %f %f %f %f %d %d %d %d %d", lineseg->id,
          lineseg->sector_id, lineseg->start.x, lineseg->start.y,
          lineseg->end.x, lineseg->end.y, lineseg->color.r, lineseg->color.g,
          lineseg->color.b, lineseg->color.a, lineseg->flags);

  if (lineseg->portal != NULL) {
    fprintf(fptr, " %d %d %d\n", lineseg->portal->output_id,
            lineseg->portal->output_sector_id, lineseg->portal->flipped);
  } else {
    fprintf(fptr, "\n");
  }
}

void write_sector(FILE *fptr, Sector sector) {
  fprintf(fptr, "sec %d %d %d %d %d %d %d %d %d %d %d\n", sector.id,
          sector.line_count, sector.ceil_color.r, sector.ceil_color.g,
          sector.ceil_color.b, sector.ceil_color.a, sector.floor_color.r,
          sector.floor_color.g, sector.floor_color.b, sector.floor_color.a,
          sector.light_modifer);

  for (int i = 0; i < sector.line_count; i++) {
    write_line(fptr, &sector.lines[i]);
  }
}
int save_map(char *name, Sector *sectors, int count) {
  // create the path
  char *path = malloc(strlen(name) + 10 * sizeof(char));
  sprintf(path, "maps/%s.map", name);

  if (path == NULL) {
    printf("Failed to set path!");
    return 0;
  }

  // create the file
  FILE *fptr = fopen(path, "w");

  // free the path as it isnt needed anymore
  free(path);

  if (fptr == NULL) {
    printf("Failed to save map!");
    return 0;
  }

  // now that the file is created write the data to it
  fprintf(fptr, "!MAP\n");

  for (int i = 0; i < count; i++) {
    fprintf(fptr, "\n");
    write_sector(fptr, sectors[i]);
  }

  printf("Saved map as %s.map\n", name);
  return 1;
}

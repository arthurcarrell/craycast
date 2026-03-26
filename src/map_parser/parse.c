#include "../state.h"
#include <SDL3/SDL_events.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_sector(char **tokens) {
  int id = atoi(tokens[1]);
  int line_count = atoi(tokens[2]);
  rgba ceil_color = (rgba){atoi(tokens[3]), atoi(tokens[4]), atoi(tokens[5]), atoi(tokens[6])};

  rgba floor_color = (rgba){atoi(tokens[7]), atoi(tokens[8]), atoi(tokens[9]), atoi(tokens[10])};

  int light = atoi(tokens[11]);

  state.sectors[id] = (Sector){id, calloc(100, sizeof(LineSegment)), line_count, 0, 20, floor_color, ceil_color, light};
  state.sector_count++;
  printf("created sector: id: %d line_count: %d light: %d\n", id, line_count, light);
}

void create_line(char **tokens) {
  int id = atoi(tokens[1]);
  int sector_id = atoi(tokens[2]);
  vec2f start = {atof(tokens[3]), atof(tokens[4])};
  vec2f end = {atof(tokens[5]), atof(tokens[6])};
  rgba color = {atoi(tokens[7]), atoi(tokens[8]), atoi(tokens[9]), atoi(tokens[10])};
  int flags = atoi(tokens[11]);

  // check the flags if it is a portal or portal exit
  if (flags & LINE_FLAG_PORTAL || flags & LINE_FLAG_PORTAL_EXIT) {
    int output_line = atoi(tokens[12]);
    int output_sect = atoi(tokens[13]);
    int flipped = atoi(tokens[14]);

    // create a portal
    Portal *portal = malloc(sizeof(Portal));
    *portal = (Portal){output_line, output_sect, flipped};
    state.sectors[sector_id].lines[id] = (LineSegment){start, end, color, id, sector_id, flags, portal};
  } else {
    state.sectors[sector_id].lines[id] = (LineSegment){start, end, color, id, sector_id, flags, NULL};
  }
  printf("created line - ID: %d sector_id: %d start: (%f,%f) end: (%f,%f) - "
         "color: (%d,%d,%d,%d)\n",
         id, sector_id, start.x, start.y, end.x, end.y, color.r, color.g, color.b, color.a);
}
void handle_line(char *line) {
  // split the line into spaces, as that is what is used for the delimiter
  char *token = strtok(line, " ");
  char **tokens = NULL;

  int count = 0;
  while (token != NULL) {
    tokens = realloc(tokens, sizeof(char *) * ++count);
    if (tokens == NULL) {
      fprintf(stderr, "Could not reallocate!");
    }

    tokens[count - 1] = token;
    token = strtok(NULL, " ");
  }

  // TODO: stuff
  if (strcmp(tokens[0], "sec") == 0) {
    create_sector(tokens);
  } else if (strcmp(tokens[0], "lineseg") == 0) {
    create_line(tokens);
  } else if (strcmp(tokens[0], "spwn") == 0) {
    state.start.pos = (vec2f){atof(tokens[1]), atof(tokens[2])};
    state.player.pos = state.start.pos;
  }

  // free the line
  free(tokens);
}

int load_map(char *name) {
  // create the path
  char *path = malloc((strlen(name) + 6) * sizeof(char));
  sprintf(path, "maps/%s", name);

  // load the file from the path
  FILE *fptr = fopen(path, "r");

  // free the path
  free(path);

  if (fptr == NULL) {
    fprintf(stderr, "Map file doesnt exist!");
    return 1;
  }
  char line[1000];
  while (fgets(line, 1000, fptr)) {
    handle_line(line);
  }

  printf("Loaded map: %s\n", name);
  fclose(fptr);
  return 0;
}

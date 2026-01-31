#include "../state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_sector(char **tokens) {
  int id = atoi(tokens[1]);
  int line_count = atoi(tokens[2]);
  rgba ceil_color = (rgba){atoi(tokens[3]), atoi(tokens[4]), atoi(tokens[5]),
                           atoi(tokens[6])};

  rgba floor_color = (rgba){atoi(tokens[7]), atoi(tokens[8]), atoi(tokens[9]),
                            atoi(tokens[10])};

  int light = atoi(tokens[11]);

  state.sectors[id] =
      (Sector){id, NULL, line_count, 0, 20, floor_color, ceil_color, light};
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
  }

  // free the line
  free(tokens);
}

int load_map(char *name) {
  // create the path
  char *path = malloc((strlen(name) + 10) * sizeof(char));
  sprintf(path, "maps/%s.map", name);

  // load the file from the path
  FILE *fptr = fopen(path, "r");

  // free the path
  free(path);

  if (fptr == NULL) {
    fprintf(stderr, "Could not read file!");
    return 1;
  }
  char line[1000];
  while (fgets(line, 1000, fptr)) {
    handle_line(line);
  }

  fclose(fptr);
  return 0;
}

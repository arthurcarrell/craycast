#include "title.h"
#include "font.h"
#include "map_parser/parse.h"
#include "state.h"
#include "utils.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

Maps maps;

char **get_all_maps(int *size) {
  DIR *directory;
  struct dirent *entry;
  // this function makes my program incompatible with Windows systems. Still works on Linux and Mac so thats good enough
  directory = opendir("maps"); // if I move program to bin then i need to add .. at the start

  // return variables
  char **results = NULL;
  *size = 0;

  if (directory == NULL) {
    // program failed to open directory!
    fprintf(stderr, "Failed to open map directory!");
    return NULL;
  }

  // opening the directory was a success, go through each file with a '.map' extension, and format the name
  while ((entry = readdir(directory)) != NULL) {
    // get the last 4 letters, if it is .map then its valid
    char *name = entry->d_name;
    char *extension = strrchr(name, '.');

    if (extension != NULL && strcmp(extension, ".map") == 0) {
      printf("found map: %s\n", name);

      // reallocate the array to add this to the list
      char **newres = realloc(results, (*size + 1) * sizeof(char *));

      // check if realloc failed
      if (newres == NULL) {
        fprintf(stderr, "Failed to reallocate map array!");
        closedir(directory);
        return results;
      }

      results = newres;

      results[*size] = strdup(name);
      (*size)++;
    }
  }

  closedir(directory);
  return results;
}

void strreplace(char *string, char old, char new) {
  for (int i = 0; i < strlen(string); i++) {
    if (string[i] == old) {
      string[i] = new;
    }
  }
}

char *beautify_string(char *string) {
  char *copy = malloc((strlen(string) + 1) * sizeof(char));
  strcpy(copy, string);
  strreplace(copy, '_', ' ');

  // remove ending
  char *dot = strrchr(copy, '.');
  if (dot && strcmp(dot, ".map") == 0) {
    *dot = '\0';
  }
  return copy;
}

void draw_map_options(vec2f startpos, char **foundmaps, int amount) {
  int font_size = 2;
  int has_selected = 0;
  // show all the map options here
  for (int i = 0; i < amount; i++) {
    // get the string to draw
    char *string = beautify_string(foundmaps[i]);
    int offset = get_text_width(strlen(string), font_size) / 2;
    vec2f finalpos = startpos;
    finalpos.x -= offset;

    rgba color = (rgba){255, 255, 255, 255};
    if (get_distance(state.mouse.pos, (vec2f){finalpos.x + offset, finalpos.y + (11 * font_size / 2)}) < 20 &&
        !has_selected) {
      color = (rgba){255, 255, 0, 255};
      has_selected = 1;
      maps.last_selected = i;
    }

    write_string(string, finalpos, color, font_size);
    startpos.y += 10 * font_size + 7;

    free(string);
  }

  if (has_selected == 0) {
    maps.last_selected = -1;
  }
}

void title_on_click() {
  // get the last selected map and load it
  if (maps.last_selected != -1) {
    printf("selected map: %s\n", maps.maps[maps.last_selected]);
    load_map(maps.maps[maps.last_selected]);
    state.in_menu = 0;
  }
}

void title_init() {
  maps.amount = 0;
  maps.maps = get_all_maps(&maps.amount);
}

void title_destroy() {
  for (int i = 0; i < maps.amount; i++) {
    free(maps.maps[i]);
  }
  free(maps.maps);
  maps.amount = 0;
}
void render_title() {
  char *title = "c raycast experiment";
  write_string(title, (vec2f){WINDOW_WIDTH / 2 - (get_text_width(strlen(title), 3) / 2), 100},
               (rgba){255, 255, 255, 255}, 3);

  char *mapselect = "select a map:";
  write_string(mapselect, (vec2f){WINDOW_WIDTH / 2 - (get_text_width(strlen(mapselect), 2) / 2), 160},
               (rgba){255, 255, 255, 255}, 2);

  // draw each map
  draw_map_options((vec2f){WINDOW_WIDTH / 2, 200}, maps.maps, maps.amount);
}

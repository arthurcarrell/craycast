#ifndef TITLE_H
#define TITLE_H

#include <stdlib.h>

typedef struct {
  char **maps;
  int amount;
  int last_selected;
} Maps;

extern Maps maps;

void render_title();
void title_init();
void title_destroy();
void title_on_click();
#endif // !TITLE_H

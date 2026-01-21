#ifndef EDITOR_H
#define EDITOR_H

#include "utils.h"

typedef struct {
  rgba color;
  int even_click;
  vec2f last_click_pos;
  int portal_mode;
  int map_mode;
  int last_line_id;
  int last_sector_id;
  int current_sector;
} Editor;

extern Editor editor;

void render_map();
void editor_init();

#endif // !TEXTURES_H

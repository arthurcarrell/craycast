#ifndef EDITOR_H
#define EDITOR_H

#include "utils.h"

#define MOUSE_TOLERANCE 1000

// editor modes
#define EDITOR_MODE_SECTOR 0
#define EDITOR_MODE_PORTAL 1
#define EDITOR_MODE_STARTPOS 2
#define EDITOR_MODE_PLACING_POINTS 3
#define EDITOR_MODE_LINE 4

typedef struct {
  rgba color;
  int map_mode;
  int mode;
  int current_sector;
  vec2f points_placed[20];
  int point_count;
} Editor;

extern Editor editor;

void render_map();
void editor_init();
void editor_on_click();
void editor_keypress(int key);

#endif // !TEXTURES_H

#ifndef STATE_H
#define STATE_H

#include "actor.h"
#include "line.h"
#include "raycast.h"
#include "sector.h"
#include "utils.h"
#include <SDL3/SDL.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

typedef struct {
  struct {
    vec2f pos;
    uint32_t canary;
  } mouse;

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  LineSegment *lines;
  Sector *sectors;
  Actor player;
  int sector_count;
  int line_count;

  int quit;
  float delta;

  struct {
    vec2f pos;
    float rot;
    float fov;
    int dist;
    Raycast result[WINDOW_WIDTH];
  } camera;

} State;

extern State state;

void state_init();
void state_destroy();

#endif

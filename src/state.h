#ifndef STATE_H
#define STATE_H

#include "line.h"
#include "raycast.h"
#include "utils.h"
#include <SDL3/SDL.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

typedef struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  Line lines[100]; // will the program segfault after 100 lines are made? yes.
                   // this is a prototype so thats fine for now
  int line_count;

  struct {
    vec2f pos;
    float rot;
    float fov;
    int dist;
    Raycast result[WINDOW_WIDTH];
  } camera;

  struct {
    vec2f pos;
  } mouse;

  bool quit;
  float delta;

} State;

extern State state;

void state_init();

#endif

#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define WINDOW_WIDTH 720
#define WINDOW_HEIGHT 480

static struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  Line lines[10];
  int line_count;

  struct {
    vec2f pos;
    float rot;
  } camera;

  bool quit;
} state;

void init_sdl() {
  state.quit = false;
  state.line_count = 0;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    // something broke
    printf("Failed to initalise SDL: %s\n", SDL_GetError());
  }

  // window and renderer
  SDL_CreateWindowAndRenderer("craycast", WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                              &state.window, &state.renderer);
}

void destroy_sdl() {
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}

// Main functions
void init() {
  state.lines[0] = (Line){
      .start = (vec2f){WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 - 100},
      .end = (vec2f){WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2 + 100},
      .color = (rgba){255, 255, 255, 255},
  };

  state.line_count = 1;

  state.camera.pos = (vec2f){50, WINDOW_HEIGHT / 2};
  state.camera.rot = 0.0;
}

// raycast
vec2f raycast(vec2f pos, float rot, int distance, int *success) {
  // create a line
  vec2f end = (vec2f){.x = pos.x + distance * cosf(rot),
                      .y = pos.y + distance * sinf(rot)};
  Line ray = (Line){pos, end, {255, 0, 0, 255}};
  for (int i = 0; i < state.line_count; i++) {
    // check if lines intersect
    vec2f result = get_line_intersections(&ray, &state.lines[i], success);
    if (*success) {
      SDL_SetRenderDrawColor(state.renderer, 0, 255, 0, 255);
      SDL_RenderLine(state.renderer, ray.start.x, ray.start.y, result.x,
                     result.y);
      return result;
    }
    SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);
    SDL_RenderLine(state.renderer, ray.start.x, ray.start.y, ray.end.x,
                   ray.end.y);
  }

  return (vec2f){0, 0};
}
void update() {
  SDL_GetMouseState(&state.camera.pos.x, &state.camera.pos.y);

  float rot = state.camera.rot; // -50 degrees
  for (int i = 0; i < 360; i++) {
    int res;
    raycast(state.camera.pos, rot, 10000, &res);
    rot += 0.01745329; // 1 deg
  }
}
void render() {
  // for each line, draw one
  for (int i = 0; i < 10 - 1; i++) {
    Line line = state.lines[i];
    SDL_SetRenderDrawColor(state.renderer, line.color.r, line.color.g,
                           line.color.b, line.color.a);
    SDL_RenderLine(state.renderer, line.start.x, line.start.y, line.end.x,
                   line.end.y);
  }
}

int main(int argc, char *argv[]) {
  init_sdl();
  init();
  SDL_Event event;
  while (!state.quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        state.quit = true;
        break;
      }
    }
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
    SDL_RenderClear(state.renderer);
    update();
    render();
    SDL_RenderPresent(state.renderer);

    SDL_Delay(
        10); // bandaid fix, I should implement an actual FPS cap but I was
             // literally hearing whistling while this used 100% of my CPU
  }
  destroy_sdl();
  return 0;
}

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
  Line lines[100]; // will the program segfault after 100 lines are made? yes.
                   // this is a prototype so thats fine for now
  int line_count;

  struct {
    vec2f pos;
    float rot;
  } camera;

  struct {
    vec2f pos;
    vec2f prev_pos; // used to get the first clicks place
    bool even_click;
  } mouse;

  bool quit;
  float delta;
} state;

void init_sdl() {
  state.quit = false;

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
  // these all need to be instantiated as otherwise C uses garbage values that
  // could literally be anything
  state.line_count = 0;
  state.camera.pos = (vec2f){50, WINDOW_HEIGHT / 2};
  state.camera.rot = 0.0;

  state.mouse.pos = (vec2f){0, 0};
  state.mouse.prev_pos = state.mouse.pos;
  state.mouse.even_click = true;

  state.delta = 0;
}

// raycast
vec2f raycast(vec2f pos, float rot, int distance, int *success) {
  // create a line
  vec2f end = (vec2f){.x = pos.x + distance * cosf(rot),
                      .y = pos.y + distance * sinf(rot)};
  Line ray = (Line){pos, end, {255, 0, 0, 255}};

  float closest = distance;
  vec2f closest_vector = end;
  *success = 0;

  for (int i = 0; i < state.line_count; i++) {
    // check if lines intersect
    int found;
    vec2f result = get_line_intersections(&ray, &state.lines[i], &found);
    if (found) {
      float current_distance = get_distance(pos, result);
      // check if closest, if so, overwrite
      if (current_distance < closest) {
        closest_vector = result;
        closest = current_distance;
        *success = 1;
      }
    }
  }

  if (*success) {
    SDL_SetRenderDrawColor(state.renderer, 0, 255, 0, 255);
    SDL_RenderLine(state.renderer, pos.x, pos.y, closest_vector.x,
                   closest_vector.y);
    return closest_vector;
  }
  SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);
  SDL_RenderLine(state.renderer, ray.start.x, ray.start.y, ray.end.x,
                 ray.end.y);

  return end;
}
void update() {
  state.camera.pos = state.mouse.pos;
  float rot = state.camera.rot;
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

void event_mouse_down() {
  if (!state.mouse.even_click) {
    state.lines[state.line_count] =
        (Line){state.mouse.prev_pos, state.mouse.pos, {255, 255, 255, 255}};
    state.line_count++;
  }
  // done
  state.mouse.prev_pos = state.mouse.pos;
  state.mouse.even_click = !state.mouse.even_click;
}

int main(int argc, char *argv[]) {
  init_sdl();
  init();
  SDL_Event event;
  while (!state.quit) {

    // frame begins
    float start = SDL_GetTicks();

    // input comes before the events
    SDL_GetMouseState(&state.mouse.pos.x, &state.mouse.pos.y);

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        state.quit = true;
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        event_mouse_down();
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

    // frame ends
    state.delta = SDL_GetTicks() - start;
  }
  destroy_sdl();
  return 0;
}

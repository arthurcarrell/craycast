#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

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
    float fov;
    int dist;
    Raycast result[WINDOW_WIDTH];
  } camera;

  struct {
    vec2f pos;
    vec2f prev_pos; // used to get the first clicks place
    bool even_click;
  } mouse;

  bool quit;
  float delta;

  bool map_mode;
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
  state.camera.fov = M_PI / 3.0f;
  state.camera.dist = 10000;

  state.mouse.pos = (vec2f){0, 0};
  state.mouse.prev_pos = state.mouse.pos;
  state.mouse.even_click = true;

  state.delta = 0;
  state.map_mode = false;
}

// raycast
Raycast raycast(vec2f pos, float rot, int distance) {
  // create a line
  vec2f end = add_direction(pos, rot, distance);
  Line ray = (Line){pos, end, {255, 0, 0, 255}};

  float closest = distance;
  vec2f closest_vector = end;
  int success = 0;

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
        success = 1;
      }
    }
  }

  if (success) {
    if (state.map_mode) {
      SDL_SetRenderDrawColor(state.renderer, 0, 255, 0, 255);
      SDL_RenderLine(state.renderer, pos.x, pos.y, closest_vector.x,
                     closest_vector.y);
    }
    return (Raycast){1, closest_vector, closest};
  }
  if (state.map_mode) {
    SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);
    SDL_RenderLine(state.renderer, ray.start.x, ray.start.y, ray.end.x,
                   ray.end.y);
  }

  return (Raycast){.hit = 0};
}
void update() {}

void render_map() {
  // draw the raycast rays
  for (int i = -1; i < WINDOW_WIDTH; i++) {
    float rot = state.camera.rot - state.camera.fov / 2.0 +
                (i / (float)WINDOW_WIDTH) * state.camera.fov;
    raycast(state.camera.pos, rot, state.camera.dist);
  }

  // for each line, draw one
  for (int i = 0; i < state.line_count; i++) {
    Line line = state.lines[i];
    SDL_SetRenderDrawColor(state.renderer, line.color.r, line.color.g,
                           line.color.b, line.color.a);
    SDL_RenderLine(state.renderer, line.start.x, line.start.y, line.end.x,
                   line.end.y);
  }
}

void render_world() {
  // do a raycast for each pixel on the screen
  for (int i = 0; i < WINDOW_WIDTH; i++) {
    float rot = state.camera.rot - state.camera.fov / 2.0 +
                (i / (float)WINDOW_WIDTH) * state.camera.fov;
    Raycast result = raycast(state.camera.pos, rot, state.camera.dist);
    if (result.hit) {
      // correct distance to remove the fisheye
      result.distance = result.distance * cosf(rot - state.camera.rot);
    }
    state.camera.result[i] = result;
  }

  int tile_size = 20;
  float projection_dist = (WINDOW_WIDTH / 2.0) / tanf(state.camera.fov / 2.0);
  // run through each raycast and draw a line on the screen for it.
  for (int i = 0; i < WINDOW_WIDTH; i++) {
    Raycast result = state.camera.result[i];
    float height = (tile_size / result.distance) * projection_dist;

    int start = WINDOW_HEIGHT / 2 - height / 2;
    int end = WINDOW_HEIGHT / 2 + height / 2;

    int darkness = clampf(result.distance / 1.5, 255, 0);
    SDL_SetRenderDrawColor(state.renderer, 255 - darkness, 255 - darkness,
                           255 - darkness, 255);
    SDL_RenderLine(state.renderer, i, start, i, end);
  }
}

void render() {
  if (state.map_mode) {
    render_map();
  } else {
    render_world();
  }
}

void event_mouse_down() {
  if (state.map_mode) {
    if (!state.mouse.even_click) {
      state.lines[state.line_count] =
          (Line){state.mouse.prev_pos, state.mouse.pos, {255, 255, 255, 255}};
      state.line_count++;
    }
    // done
    state.mouse.prev_pos = state.mouse.pos;
    state.mouse.even_click = !state.mouse.even_click;
  }
}

void get_keyboard_input() {
  // get the key pressed
  const bool *key_states = SDL_GetKeyboardState(0);

  float movespeed = 0.25;
  float look_sensitivity = 0.25;
  if (key_states[SDL_SCANCODE_LEFT]) {
    state.camera.rot -= 0.01745329 * state.delta * look_sensitivity;
  }
  if (key_states[SDL_SCANCODE_RIGHT]) {
    state.camera.rot += 0.01745329 * state.delta * look_sensitivity;
  }
  if (key_states[SDL_SCANCODE_W]) {
    state.camera.pos = add_direction(state.camera.pos, state.camera.rot,
                                     movespeed * state.delta);
  }
  if (key_states[SDL_SCANCODE_S]) {
    state.camera.pos = add_direction(state.camera.pos, state.camera.rot,
                                     -(movespeed * state.delta));
  }
  if (key_states[SDL_SCANCODE_A]) {
    state.camera.pos =
        add_direction(state.camera.pos, state.camera.rot + 1.570796,
                      -(movespeed * state.delta));
  }
  if (key_states[SDL_SCANCODE_D]) {
    state.camera.pos =
        add_direction(state.camera.pos, state.camera.rot - 1.570796,
                      -(movespeed * state.delta));
  }
}

void event_key_down(int key) {
  if (key == SDLK_Z) {
    state.map_mode = !state.map_mode;
  }
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
      case SDL_EVENT_KEY_DOWN:
        event_key_down(event.key.key);
      }
    }
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
    SDL_RenderClear(state.renderer);
    get_keyboard_input();
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

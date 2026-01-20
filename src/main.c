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
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "editor.h" // has all the editor functions
#include "framebuf.h"
#include "input.h" // gets input
#include "line.h"
#include "raycast.h" // the raycaster
#include "sdl.h"     // Create and destroy SDL
#include "state.h"   // 'god struct' and 'god macros'
#include "utils.h"   // math functions, structs and other misc stuff

// the minimum amount of ms to wait - 16 is around 60 fps
// this exists as without it this will try and render as much as it can, which
// ends up using like all of your cpu
#define MIN_MS 16

// Main functions
void init() {
  sdl_init();
  // framebuffer
  state.renderer = framebuf_init(&framebuf, &state, (rgba){0, 0, 0, 255});
  // state
  state_init();
  // editor
  editor_init();
}

void destroy() {
  sdl_destroy();
  framebuf_destroy(&framebuf);
}

void update() {}

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

  // store the results in arrays to be handed to the framebuffer
  int top[WINDOW_WIDTH];
  int bottom[WINDOW_WIDTH];
  uint32_t wallcolor[WINDOW_WIDTH];

  int tile_size = 20;
  float projection_dist = (WINDOW_WIDTH / 2.0) / tanf(state.camera.fov / 2.0);
  // run through each raycast and draw a line on the screen for it.
  for (int i = 0; i < WINDOW_WIDTH; i++) {
    Raycast result = state.camera.result[i];
    if (result.hit) {

      float height = (tile_size / result.distance) * projection_dist;

      int start = WINDOW_HEIGHT / 2.0 - height / 2;
      int end = WINDOW_HEIGHT / 2.0 + height / 2;

      rgba color = state.lines[result.line_id].color;

      int darkness = clampf(result.distance / 1.5, 255, 0);

      int r = clamp(color.r - darkness, 255, 0);
      int g = clamp(color.g - darkness, 255, 0);
      int b = clamp(color.b - darkness, 255, 0);

      // store the results into an array so that the framebuffer doesnt have a
      // hard time
      top[i] = start > 0 ? start : 0;
      bottom[i] = end >= WINDOW_HEIGHT ? WINDOW_HEIGHT - 1 : end;
      wallcolor[i] = rgba_to_int((rgba){r, g, b, 255});

    } else {
      top[i] = -1;
      bottom[i] = -1;
      wallcolor[i] = 0;
    }
  }
  // give the framebuffer the wall info
  framebuf_column_optimised(&framebuf, top, bottom, wallcolor, WINDOW_WIDTH);
}

void render() {
  if (editor.map_mode) {
    render_map();
  } else {
    render_world();
  }
}

// Where the program actually runs, keeps going until quit is true
int main(int argc, char *argv[]) {
  init();
  SDL_Event event;
  while (!state.quit) {

    // frame begins
    float start = SDL_GetTicks();

    // input comes before the events
    SDL_GetMouseState(&state.mouse.pos.x, &state.mouse.pos.y);

    // do events
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

    // --- MAIN ---

    // Clear the framebuffer
    framebuf_clear(&framebuf);

    // Do rendering and gameplay
    get_keyboard_input();
    update();
    render();

    // Draw the render to the screen
    framebuf_screen(&framebuf, state.window);

    // frame ends
    state.delta = SDL_GetTicks() - start;
    // if the time it took to draw that frame is below MIN_MS, then wait that
    // amount of time
    if (state.delta < MIN_MS) {
      SDL_Delay(MIN_MS - state.delta);
      state.delta = MIN_MS;
    }
  }

  // The program has now ended, destroy SDL
  destroy();
  return 0;
}

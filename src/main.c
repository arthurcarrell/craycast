#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <assert.h>
#include <float.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "actor.h"
#include "editor.h" // has all the editor functions
#include "font.h"
#include "framebuf.h" // the framebuffer
#include "input.h"    // gets input
#include "map_parser/parse.h"
#include "map_parser/save.h"
#include "sdl.h" // Create and destroy SDL
#include "sector.h"
#include "state.h" // 'god struct' and 'god macros'
#include "title.h"
#include "utils.h" // math functions, structs and other misc stuff
#include "world.h"

// TODO: either stop doing more optimisations - 90-100 fps is good when printing
// text every frame and also drawing a complete frame at 1920x1080 OR attempt to
// scale up the surface to make the game slightly lower res (more 90s) but a lot
// faster

// the minimum amount of ms to wait - 16 is around 60 fps
// this exists as without it this will try and render as much as it can, which
// ends up using like all of your cpu
#define MIN_MS 16

// Main functions
void init() {
  state_init();
  sdl_init();
  // framebuffer
  state.renderer = framebuf_init(&framebuf, state.window, (rgba){0, 0, 0, 255});
  // editor
  font_init(state.renderer);
  editor_init();
  title_init();
}

void destroy() {
  printf("Cleaning up\n");
  save_map(maps.maps[maps.last_selected], state.sectors, state.sector_count, state.start.pos, state.end.pos);
  sdl_destroy();
  font_destroy();
  framebuf_destroy(&framebuf);
  state_destroy();
  title_destroy();
  printf("Clean up complete\n");
}

void update() {
  state.camera.pos = state.player.pos;
  state.camera.rot = state.player.rot;
}

void render() {
  if (state.in_menu) {
    render_title();
  } else if (editor.map_mode) {
    render_map();
  } else {
    render_world();
  }
}

// Where the program actually runs, keeps going until quit is true
int main(int argc, char *argv[]) {
  init();
  while (!state.quit) {

    // frame begins
    float start = SDL_GetTicks();

    // input comes before the events
    SDL_GetMouseState(&state.mouse.pos.x, &state.mouse.pos.y);
    // do events
    SDL_Event event;
    SDL_zero(event);
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
        break;
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

    // printf("fps: %.2f \n", (1000.0 / state.delta));
  }

  // The program has now ended, destroy SDL
  destroy();
  return 0;
}

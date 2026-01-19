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
#include <stdbool.h>

#include "editor.h"  // has all the editor functions
#include "raycast.h" // the raycaster
#include "sdl.h"     // Create and destroy SDL
#include "state.h"   // 'god struct' and 'god macros'
#include "utils.h"   // math functions, structs and other misc stuff

// Main functions
void init() {
  // state
  state_init();
  // editor
  editor_init();
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

  int tile_size = 20;
  float projection_dist = (WINDOW_WIDTH / 2.0) / tanf(state.camera.fov / 2.0);
  // run through each raycast and draw a line on the screen for it.
  for (int i = 0; i < WINDOW_WIDTH; i++) {
    Raycast result = state.camera.result[i];
    float height = (tile_size / result.distance) * projection_dist;

    int start = WINDOW_HEIGHT / 2.0 - height / 2;
    int end = WINDOW_HEIGHT / 2.0 + height / 2;

    rgba color = state.lines[result.line_id].color;

    int darkness = clampf(result.distance / 1.5, 255, 0);

    int r = clamp(color.r - darkness, 255, 0);
    int g = clamp(color.g - darkness, 255, 0);
    int b = clamp(color.b - darkness, 255, 0);

    SDL_SetRenderDrawColor(state.renderer, r, g, b, color.a);
    SDL_RenderLine(state.renderer, i, start, i, end);
  }
}

void render() {
  if (editor.map_mode) {
    render_map();
  } else {
    render_world();
  }
}

// Ran every time a mouse button is pressed
void event_mouse_down() {
  if (editor.map_mode) {
    if (!editor.portal_mode) {
      if (!editor.even_click) {
        Line line = create_render_line(editor.last_click_pos, state.mouse.pos,
                                       editor.color);
        line.id = state.line_count;
        state.lines[state.line_count++] = line;
      }

      editor.last_click_pos = state.mouse.pos;
    } else if (editor.map_mode && editor.portal_mode) {
      for (int i = 0; i < state.line_count; i++) {
        Line *line = &state.lines[i];
        if (is_on_line(state.mouse.pos, *line, 1000)) {
          if (editor.even_click) {
            editor.last_line_id = line->id;
            editor.last_click_pos = line->end;
          } else {
            // turn the previous line into a portal and the current line into a
            // portal exit
            Line *prev = &state.lines[editor.last_line_id];

            if (prev != line) {
              // check if changing portal target
              if (prev->flags & LINE_FLAG_PORTAL) {
                // yes, so strip the portal output flag from the portal output
                state.lines[prev->portal.output_id].flags &=
                    ~LINE_FLAG_PORTAL_EXIT;

                // will be overwritten but just in case
                prev->portal.output_id = 0;
              }
              prev->portal.output_id = line->id;
              prev->flags |= LINE_FLAG_PORTAL;

              line->flags |= LINE_FLAG_PORTAL_EXIT;
              line->portal.output_id = prev->id;
            }
            // disable portal mode
            editor.portal_mode = 0;
            editor.last_click_pos = state.mouse.pos;
          }
        }
      }
    }
    editor.even_click = !editor.even_click;
  }
}

// Keyboard input
void get_keyboard_input() {
  // get the key pressed
  const bool *key_states = SDL_GetKeyboardState(0);

  float movespeed = 0.25 / 2;
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
        add_direction(state.camera.pos, state.camera.rot + NINETY_DEGINRAD,
                      -(movespeed * state.delta));
  }
  if (key_states[SDL_SCANCODE_D]) {
    state.camera.pos =
        add_direction(state.camera.pos, state.camera.rot - NINETY_DEGINRAD,
                      -(movespeed * state.delta));
  }
}

void event_key_down(int key) {
  if (editor.map_mode) {
    if (key == SDLK_W) {
      editor.color = (rgba){255, 255, 255, 255};
    } else if (key == SDLK_R) {
      editor.color = (rgba){255, 0, 0, 255};
    } else if (key == SDLK_Y) {
      editor.color = (rgba){255, 255, 0, 255};
    } else if (key == SDLK_G) {
      editor.color = (rgba){0, 255, 0, 255};
    } else if (key == SDLK_T) {
      editor.color = (rgba){0, 255, 255, 255};
    } else if (key == SDLK_B) {
      editor.color = (rgba){0, 0, 255, 255};
    } else if (key == SDLK_P) {
      editor.color = (rgba){255, 0, 255, 255};
    } else if (key == SDLK_X) {
      editor.portal_mode = !editor.portal_mode;
      editor.even_click = 1;
    } else if (key == SDLK_F && editor.last_line_id != -1) {
      if (state.lines[editor.last_line_id].flags & LINE_FLAG_PORTAL_EXIT) {
        state.lines[editor.last_line_id].portal.flipped =
            !state.lines[editor.last_line_id].portal.flipped;
      }
    }
  }

  if (key == SDLK_Z) {
    editor.map_mode = !editor.map_mode;
    editor.portal_mode = 0;
  }
}

// Where the program actually runs, keeps going until quit is true
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

  // The program has now ended, destroy SDL
  destroy_sdl();
  return 0;
}

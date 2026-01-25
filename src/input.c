#include "actor.h"
#include "editor.h"
#include "line.h"
#include "sector.h"
#include "state.h"
#include "utils.h"
#include <SDL3/SDL_keycode.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

// Ran every time a mouse button is pressed
void event_mouse_down() {
  if (editor.map_mode) {
    editor_on_click();
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
    actor_move(&state.player, state.camera.rot, movespeed * state.delta);
  }
  if (key_states[SDL_SCANCODE_S]) {
    actor_move(&state.player, state.camera.rot, -(movespeed * state.delta));
  }
  if (key_states[SDL_SCANCODE_A]) {
    actor_move(&state.player, state.camera.rot + NINETY_DEGINRAD,
               -(movespeed * state.delta));
  }
  if (key_states[SDL_SCANCODE_D]) {
    actor_move(&state.player, state.camera.rot - NINETY_DEGINRAD,
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
    } else if (key == SDLK_F) {

      if (get_sector_of_point(state.mouse.pos) != NULL) {
        LineSegment *line = get_line_at_point(
            state.mouse.pos, get_sector_of_point(state.mouse.pos)->lines,
            get_sector_of_point(state.mouse.pos)->line_count, 1000);

        if (line != NULL && line->portal != NULL) {
          line->portal->flipped = !line->portal->flipped;
        }
      }
    } else if (key == SDLK_0) {
      printf("Changed editor sector: %d\n", ++editor.current_sector);
    } else if (key == SDLK_9 && editor.current_sector > 0) {
      printf("Changed editor sector: %d\n", --editor.current_sector);
    }
  }

  if (key == SDLK_Z) {
    editor.map_mode = !editor.map_mode;
    editor.portal_mode = 0;
  }
}

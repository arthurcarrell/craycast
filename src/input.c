#include "actor.h"
#include "editor.h"
#include "state.h"
#include "title.h"
#include "utils.h"
#include <SDL3/SDL_keycode.h>

// Ran every time a mouse button is pressed
void event_mouse_down() {
  if (state.in_menu) {
    title_on_click();
  } else if (editor.map_mode) {
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
    state.player.rot -= 0.01745329 * state.delta * look_sensitivity;
  }
  if (key_states[SDL_SCANCODE_RIGHT]) {
    state.player.rot += 0.01745329 * state.delta * look_sensitivity;
  }
  if (key_states[SDL_SCANCODE_W]) {
    actor_move(&state.player, movespeed * state.delta, state.player.rot);
  }
  if (key_states[SDL_SCANCODE_S]) {
    actor_move(&state.player, -(movespeed * state.delta), state.player.rot);
  }
  if (key_states[SDL_SCANCODE_A]) {
    actor_move(&state.player, -(movespeed * state.delta), state.player.rot + NINETY_DEGINRAD);
  }
  if (key_states[SDL_SCANCODE_D]) {
    actor_move(&state.player, -(movespeed * state.delta), state.player.rot - NINETY_DEGINRAD);
  }
}

void event_key_down(int key) {
  if (editor.map_mode) {
    editor_keypress(key);
  }
  if (key == SDLK_Z) {
    editor.map_mode = !editor.map_mode;
    editor.mode = EDITOR_MODE_SECTOR;
  }
}

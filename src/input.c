#include "editor.h"
#include "state.h"

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

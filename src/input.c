#include "editor.h"
#include "line.h"
#include "sector.h"
#include "state.h"
#include "utils.h"
#include <SDL3/SDL_keycode.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

// Ran every time a mouse button is pressed
void event_mouse_down() {
  if (editor.map_mode) {
    if (!editor.portal_mode) {
      if (!editor.even_click) {

        // Sector *sector = &state.sectors[editor.current_sector];
        /*sector->lines[sector->line_count] =
           (LineSegment){editor.last_click_pos, state.mouse.pos, editor.color,
                                                          sector->line_count,
                                                          sector->id,
                                                          0,
                                                          NULL};*/
        // sector->line_count++;
      }

      // create a new sector
      Sector *current = sector_create(5, (rgba){0, 0, 0, 255}, 20,
                                      (rgba){0, 0, 0, 255}, 0, 0);

      int size = 50;
      current->lines[0] = (LineSegment){
          .start = (vec2f){state.mouse.pos.x - size, state.mouse.pos.y - size},
          .end = (vec2f){state.mouse.pos.x + size, state.mouse.pos.y - size},
          .color = editor.color,
          current->line_count++,
          current->id,
          0,
          NULL};
      current->lines[1] = (LineSegment){
          .start = (vec2f){state.mouse.pos.x + size, state.mouse.pos.y - size},
          .end = (vec2f){state.mouse.pos.x + size, state.mouse.pos.y + size},
          .color = editor.color,
          current->line_count++,
          current->id,
          0,
          NULL};

      current->lines[2] = (LineSegment){
          .start = (vec2f){state.mouse.pos.x - size, state.mouse.pos.y + size},
          .end = (vec2f){state.mouse.pos.x + size, state.mouse.pos.y + size},
          .color = editor.color,
          current->line_count++,
          current->id,
          0,
          NULL};
      current->lines[3] = (LineSegment){
          .start = (vec2f){state.mouse.pos.x - size, state.mouse.pos.y + size},
          .end = (vec2f){state.mouse.pos.x - size, state.mouse.pos.y - size},
          .color = editor.color,
          current->line_count++,
          current->id,
          0,
          NULL};
      current->line_count = 4;
      printf("clicky\n");
      editor.last_click_pos = state.mouse.pos;
    } else if (editor.map_mode && editor.portal_mode) {
      for (int s = 0; s < state.sector_count; s++) {
        Sector *sec = &state.sectors[s];
        for (int i = 0; i < sec->line_count; i++) {
          LineSegment *line = &sec->lines[i];
          if (is_on_line(state.mouse.pos, lineseg_line(*line), 1000)) {
            if (editor.even_click) {
              editor.last_line_id = line->id;
              editor.last_click_pos = line->end;
            } else {
              // turn the previous line into a portal and the current line into
              // a portal exit
              LineSegment *prev = &state.sectors[editor.last_sector_id]
                                       .lines[editor.last_line_id];

              if (prev != line) {
                // check if changing portal target
                if (prev->flags & LINE_FLAG_PORTAL) {
                  // yes, so strip the portal output flag from the portal output
                  sec->lines[prev->portal->output_id].flags &=
                      ~LINE_FLAG_PORTAL_EXIT;
                }

                if (prev->portal == NULL) {
                  Portal *portal = malloc(sizeof(Portal));
                  *portal = (Portal){line->id, 0};
                  prev->portal = portal;
                } else {
                  prev->portal->output_id = line->id;
                }

                prev->flags |= LINE_FLAG_PORTAL;

                if (line->portal == NULL) {
                  // create a portal struct
                  Portal *portal = malloc(sizeof(Portal));
                  *portal = (Portal){prev->portal->output_id, 0};
                  line->portal = portal;
                } else {
                  line->portal->output_id = prev->portal->output_id;
                }

                line->flags |= LINE_FLAG_PORTAL_EXIT;
              }
              // disable portal mode
              editor.portal_mode = 0;
              editor.last_click_pos = state.mouse.pos;
            }
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
        state.lines[editor.last_line_id].portal->flipped =
            !state.lines[editor.last_line_id].portal->flipped;
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

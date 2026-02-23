#include "editor.h"
#include "font.h"
#include "framebuf.h"
#include "line.h"
#include "sector.h"
#include "state.h"
#include "utils.h"
#include <SDL3/SDL_keycode.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

Editor editor;

void editor_text() {
  if (editor.mode == EDITOR_MODE_PORTAL) {
    write_string("portal mode", (vec2f){WINDOW_WIDTH - (10 * 12) - 1, 11}, (rgba){255, 255, 255, 255}, 1);
  } else if (editor.mode == EDITOR_MODE_SECTOR) {
    write_string("map mode", (vec2f){WINDOW_WIDTH - (10 * 9) - 1, 11}, (rgba){255, 255, 255, 255}, 1);
  } else if (editor.mode == EDITOR_MODE_PLACING_POINTS) {
    write_string("creating sector", (vec2f){WINDOW_WIDTH - (10 * 15) - 1, 11}, (rgba){255, 255, 255, 255}, 1);
  } else if (editor.mode == EDITOR_MODE_ENDPOS) {
    write_string("placing flag", (vec2f){WINDOW_WIDTH - (10 * 12) - 1, 11}, (rgba){255, 255, 255, 255}, 1);
  }

  int warnings = 0;
  if (state.start.pos.x == 0 && state.start.pos.y == 0) {
    write_string("a start position", (vec2f){10, WINDOW_HEIGHT - (11 * (3 - warnings))}, (rgba){255, 64, 64, 255}, 1);
    warnings++;
  }
  if (state.end.pos.x == 0 && state.end.pos.y == 0) {
    write_string("an end position", (vec2f){10, WINDOW_HEIGHT - (11 * (3 - warnings))}, (rgba){255, 64, 64, 255}, 1);
    warnings++;
  }

  if (warnings > 0) {
    write_string("your map is missing:", (vec2f){10, WINDOW_HEIGHT - (11 * 4)}, (rgba){255, 64, 64, 255}, 1);
  }
}

void create_sector_with_points() {
  // take the points from the sectors and create a sector with those
  Sector *sector = sector_create(20, (rgba){0}, 20, (rgba){0}, 0, 0);
  for (int i = 0; i < editor.point_count; i++) {
    // turn the points into LineSegments

    // get the point of the end of the line and of the next line
    vec2f point;
    if (i + 1 == editor.point_count) {
      point = editor.points_placed[0];
    } else {
      point = editor.points_placed[i + 1];
    }

    LineSegment line = (LineSegment){
        editor.points_placed[i], point, (rgba){255, 255, 255, 255}, sector->line_count, sector->id, 0, NULL};

    sector->lines[sector->line_count] = line;
    sector->line_count++;
  }

  editor.point_count = 0;
}

void editor_keypress(int key) {
  if (editor.map_mode) {
    if (key == SDLK_X) {
      if (editor.mode == EDITOR_MODE_PORTAL) {
        editor.mode = EDITOR_MODE_SECTOR;
      } else {
        editor.mode = EDITOR_MODE_PORTAL;
        editor.even_click = 1;
      }
    } else if (key == SDLK_F) {

      if (get_sector_of_point(state.mouse.pos) != NULL) {
        LineSegment *line = get_line_at_point(state.mouse.pos, get_sector_of_point(state.mouse.pos)->lines,
                                              get_sector_of_point(state.mouse.pos)->line_count, 1000);

        if (line != NULL && line->portal != NULL) {
          line->portal->flipped = !line->portal->flipped;
        }
      }
    } else if (key == SDLK_S && editor.point_count > 2 && editor.map_mode) {
      create_sector_with_points();
      printf("created sector\n");
      editor.mode = EDITOR_MODE_SECTOR;
    } else if (key == SDLK_E) {
      editor.mode = EDITOR_MODE_ENDPOS;
    } else if (key == SDLK_B) {
      editor.mode = EDITOR_MODE_STARTPOS;
    } else if (key == SDLK_L) {
      editor.mode = EDITOR_MODE_LINE;
      editor.point_count = 0;
    }
  }
}

void draw_sector_creation() {
  for (int i = 0; i < editor.point_count; i++) {
    vec2f startpos = editor.points_placed[i];
    vec2f endpos;
    if (i + 1 == editor.point_count) {
      endpos = state.mouse.pos;
    } else {
      endpos = editor.points_placed[i + 1];
    }

    framebuf_line_s(&framebuf, startpos.x, startpos.y, endpos.x, endpos.y, (rgba){128, 128, 128, 128});
  }
}

void draw_mouse_string(LineSegment line) {
  char data[100];
  sprintf(data, "sector id: %d\nline id: %d", line.sector_id, line.id);
  write_string(data, (vec2f){state.mouse.pos.x + 5, state.mouse.pos.y - 20}, (rgba){255, 255, 255, 255}, 1);
  if (editor.mode != EDITOR_MODE_PORTAL) {
    write_string("click to change line color", (vec2f){state.mouse.pos.x + 5, state.mouse.pos.y}, line.color, 1);
  } else {
    if (editor.even_click) {
      write_string("click to set as portal entrance", (vec2f){state.mouse.pos.x + 5, state.mouse.pos.y},
                   (rgba){55, 128, 128, 255}, 1);
    } else {
      write_string("click to set as portal exit", (vec2f){state.mouse.pos.x + 5, state.mouse.pos.y},
                   (rgba){255, 190, 0, 255}, 1);
    }
  }

  if (line.flags & LINE_FLAG_PORTAL_EXIT) {
    write_string("press 'f' to flip direction", (vec2f){state.mouse.pos.x + 5, state.mouse.pos.y + 10},
                 (rgba){255, 255, 255, 255}, 1);
  }
}

void render_map() {
  // draw the raycast rays
  for (int i = -1; i < WINDOW_WIDTH; i++) {
    float rot = state.camera.rot - state.camera.fov / 2.0 + (i / (float)WINDOW_WIDTH) * state.camera.fov;
    Sector *sec = get_sector_of_point(state.camera.pos);
    if (sec != NULL) {
      raycast_sec(sec, state.camera.pos, rot, state.camera.dist);
    }
  }

  // for each line, draw one
  int precision = 1000;
  rgba final_color = (rgba){0};
  for (int s = 0; s < state.sector_count; s++) {
    Sector *sec = &state.sectors[s];
    for (int i = 0; i < sec->line_count; i++) {
      LineSegment line = sec->lines[i];
      if (!is_on_line(state.mouse.pos, lineseg_line(line), precision)) {
        final_color = line.color;
        if (line.flags & LINE_FLAG_PORTAL || line.flags & LINE_FLAG_PORTAL_EXIT) {
          if (is_on_line(state.mouse.pos,
                         lineseg_line(state.sectors[line.portal->output_sector_id].lines[line.portal->output_id]),
                         precision)) {
            final_color = (rgba){255, 136, 0, 255};
          }
        }
      } else {
        final_color = (rgba){0, 174, 255, 255};
        draw_mouse_string(line);
      }
      framebuf_line_s(&framebuf, line.start.x, line.start.y, line.end.x, line.end.y, final_color);
    }
  }

  // draw a gray line to indicate a line in progress
  if (editor.mode == EDITOR_MODE_PORTAL && !editor.even_click) {
    framebuf_line_s(&framebuf, editor.last_click_pos.x, editor.last_click_pos.y, state.mouse.pos.x, state.mouse.pos.y,
                    (rgba){128, 128, 128, 128});
  }

  // visuals to help with the editor
  editor_text();
  draw_sector_creation();

  // draw the end/start pos
  if (editor.mode == EDITOR_MODE_ENDPOS || editor.mode == EDITOR_MODE_STARTPOS) {
    rgba color;
    if (get_sector_of_point(state.mouse.pos) == NULL) {
      color = (rgba){255, 0, 0, 255};

    } else {
      color = (rgba){255, 255, 0, 255};
    }
    char icon = (editor.mode == EDITOR_MODE_ENDPOS) ? 'F' : '+';
    write_character(state.mouse.pos, icon, color, 2);
  } else {
    write_character(state.end.pos, 'F', (rgba){255, 255, 0, 255}, 2);
    write_character(state.start.pos, '+', (rgba){255, 255, 0, 255}, 2);
  }

  // if drawing a line, show it
  if (editor.mode == EDITOR_MODE_LINE && editor.point_count > 0) {
    vec2f startpos = state.mouse.pos;
    vec2f endpos = editor.points_placed[0];
    rgba color;
    if (get_sector_of_point(state.mouse.pos) == NULL) {
      color = (rgba){255, 0, 0, 128};
    } else {
      color = (rgba){128, 128, 128, 128};
    }
    framebuf_line_s(&framebuf, startpos.x, startpos.y, endpos.x, endpos.y, color);
  }
}

void create_point(vec2f pos) {
  editor.points_placed[editor.point_count] = pos;
  editor.point_count++;
}

void create_portal() {
  for (int s = 0; s < state.sector_count; s++) {
    Sector *sec = &state.sectors[s];

    for (int i = 0; i < sec->line_count; i++) {
      LineSegment *line = &sec->lines[i];
      if (is_on_line(state.mouse.pos, lineseg_line(*line), MOUSE_TOLERANCE)) {
        if (editor.even_click) {
          editor.last_line_id = line->id;
          editor.last_sector_id = line->sector_id;
          editor.last_click_pos = line->end;
        } else {
          // turn the previous line into a portal and the current line into
          // a portal exit
          LineSegment *prev = &state.sectors[editor.last_sector_id].lines[editor.last_line_id];

          if (prev != line) {

            if (prev->portal == NULL) {
              Portal *portal = malloc(sizeof(Portal));
              *portal = (Portal){line->id, line->sector_id, 0};
              prev->portal = portal;
            } else {
              prev->portal->output_id = line->id;
              prev->portal->output_sector_id = line->sector_id;
            }

            prev->flags |= LINE_FLAG_PORTAL;

            if (line->portal == NULL) {
              // create a portal struct
              Portal *portal = malloc(sizeof(Portal));
              *portal = (Portal){prev->id, prev->sector_id, 0};
              line->portal = portal;
            } else {
              line->portal->output_id = prev->id;
              line->portal->output_sector_id = prev->sector_id;
            }

            line->flags |= LINE_FLAG_PORTAL_EXIT;
          }
          // disable portal mode
          editor.mode = EDITOR_MODE_SECTOR;
          editor.last_click_pos = state.mouse.pos;
        }
      }
    }
  }
}

void editor_change_line_color(LineSegment *line) {
  if (cmp_rgba(line->color, (rgba){255, 255, 255, 255})) {
    line->color = (rgba){255, 0, 0, 255};
  } else if (cmp_rgba(line->color, (rgba){255, 0, 0, 255})) {
    line->color = (rgba){0, 255, 0, 255};
  } else if (cmp_rgba(line->color, (rgba){0, 255, 0, 255})) {
    line->color = (rgba){0, 255, 255, 255};
  } else if (cmp_rgba(line->color, (rgba){0, 255, 255, 255})) {
    line->color = (rgba){0, 0, 255, 255};
  } else if (cmp_rgba(line->color, (rgba){0, 0, 255, 255})) {
    line->color = (rgba){255, 0, 255, 255};
  } else {
    line->color = (rgba){255, 255, 255, 255};
  }
}

void editor_create_line(Sector *sector) {
  if (editor.point_count == 0) {
    editor.points_placed[0] = state.mouse.pos;
    editor.point_count++;
  } else {
    sector->lines[sector->line_count] = (LineSegment){editor.points_placed[0],
                                                      state.mouse.pos,
                                                      (rgba){255, 255, 255, 255},
                                                      sector->line_count,
                                                      sector->id,
                                                      LINE_FLAG_INTERNAL,
                                                      NULL};
    sector->line_count++;
    editor.mode = EDITOR_MODE_SECTOR;
    editor.point_count = 0;
  }
}
void editor_on_click() {
  if (editor.mode == EDITOR_MODE_SECTOR) {
    int found_line = 0;
    // check if theyre touching a line
    for (int i = 0; i < state.sector_count; i++) {
      LineSegment *line =
          get_line_at_point(state.mouse.pos, state.sectors[i].lines, state.sectors[i].line_count, MOUSE_TOLERANCE);
      if (line != NULL) {
        found_line = 1;
        editor_change_line_color(line);
        printf("changing line\n");
        break;
      }
    }
    // create a new sector
    if (!found_line && editor.mode == EDITOR_MODE_SECTOR) {
      printf("clicky\n");
      editor.mode = EDITOR_MODE_PLACING_POINTS;
      create_point(state.mouse.pos);
    }
    editor.last_click_pos = state.mouse.pos;
  } else if (editor.map_mode && editor.mode == EDITOR_MODE_PORTAL) {
    create_portal();
  } else if (editor.map_mode && editor.mode == EDITOR_MODE_PLACING_POINTS) {
    create_point(state.mouse.pos);
    printf("creating point\n");
  } else if (editor.map_mode && editor.mode == EDITOR_MODE_ENDPOS && get_sector_of_point(state.mouse.pos) != NULL) {
    state.end.pos = state.mouse.pos;
    editor.mode = EDITOR_MODE_SECTOR;
  } else if (editor.map_mode && editor.mode == EDITOR_MODE_STARTPOS && get_sector_of_point(state.mouse.pos) != NULL) {
    state.start.pos = state.mouse.pos;
    editor.mode = EDITOR_MODE_SECTOR;
  } else if (editor.map_mode && editor.mode == EDITOR_MODE_LINE && get_sector_of_point(state.mouse.pos) != NULL) {
    editor_create_line(get_sector_of_point(state.mouse.pos));
  }

  editor.even_click = !editor.even_click;
}

void editor_init() {
  editor = (Editor){0};
  editor.even_click = 1;
  editor.map_mode = 1;
  editor.color = (rgba){255, 255, 255, 255};
  editor.last_click_pos = (vec2f){0, 0};
  editor.last_line_id = -1;
  editor.last_sector_id = 0;
  editor.current_sector = 0;
}

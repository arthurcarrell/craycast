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

/* --- EDITOR FUNCTIONS --- */
// Functions that actually do stuff that isnt drawing: e.g. creating sectors or recognising keypresses

// take the points inside of 'points_placed' and create a sector out of them
void create_sector_with_points() {
  // take the points from the sectors and create a sector with those
  Sector *sector = sector_create(20, (rgba){0}, 20, (rgba){0}, 0, 0);
  for (int i = 0; i < editor.point_count; i++) {
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

// create a point to go in 'points_placed'
void create_point(vec2f pos) {
  editor.points_placed[editor.point_count] = pos;
  editor.point_count++;
}

// Create a portal with entry as the entry and dest as the exit. - Calling 'dest' 'exit' was tempting but it overrides
// the exit() function, I still technically could but its bad practice.
void create_portal(LineSegment *entry, LineSegment *dest) {
  if (entry != dest) {
    if (entry->portal == NULL) {
      Portal *portal = malloc(sizeof(Portal));
      *portal = (Portal){dest->id, dest->sector_id, 0};
      entry->portal = portal;
    } else {
      entry->portal->output_id = dest->id;
      entry->portal->output_sector_id = dest->sector_id;
    }

    entry->flags |= LINE_FLAG_PORTAL;

    if (dest->portal == NULL) {
      // create a portal struct
      Portal *portal = malloc(sizeof(Portal));
      *portal = (Portal){entry->id, entry->sector_id, 0};
      dest->portal = portal;
    } else {
      dest->portal->output_id = entry->id;
      dest->portal->output_sector_id = entry->sector_id;
    }

    dest->flags |= LINE_FLAG_PORTAL_EXIT;
  }
}

void connect_lines_for_portal(int tolerance) {
  // Now I couldve done this with just get_sector_of_point and get_line_at_point, but this means your mouse
  // would always have to be in a sector to create a portal, which feels bad. So by looping through each sector and
  // checking if touching a line there it is better.
  LineSegment *line;
  for (int i = 0; i < state.sector_count; i++) {
    Sector *sec = &state.sectors[i];
    line = get_line_at_point(state.mouse.pos, sec->lines, sec->line_count, tolerance);
    if (line != NULL)
      break;
  }
  if (line == NULL) {
    return;
  }
  // clicked on a line
  if (editor.point_count <= 0) {
    // first click, so we are drawing a portal, add the point
    create_point(line->start);
  } else {
    // already clicked somewhere, so get the line at the first coordinate and then create the portal
    Sector *entry_sec = get_sector_of_point(editor.points_placed[0]);
    if (entry_sec == NULL)
      return;
    LineSegment *entry = get_line_at_point(editor.points_placed[0], entry_sec->lines, entry_sec->line_count, tolerance);
    if (entry != NULL) {
      create_portal(entry, line);
    }
    // disable portal mode
    editor.mode = EDITOR_MODE_SECTOR;
    editor.point_count = 0;
  }
}
// change the line color of the line clicked
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

// create a line that is enveloped by a sector
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

// runs when any click happens while the editor is open
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
  } else if (editor.mode == EDITOR_MODE_PORTAL) {
    connect_lines_for_portal(MOUSE_TOLERANCE);
  } else if (editor.mode == EDITOR_MODE_PLACING_POINTS) {
    create_point(state.mouse.pos);
    printf("creating point\n");
  } else if (editor.mode == EDITOR_MODE_STARTPOS && get_sector_of_point(state.mouse.pos) != NULL) {
    state.start.pos = state.mouse.pos;
    editor.mode = EDITOR_MODE_SECTOR;
  } else if (editor.mode == EDITOR_MODE_LINE && get_sector_of_point(state.mouse.pos) != NULL) {
    editor_create_line(get_sector_of_point(state.mouse.pos));
  }
}

// runs when any key is pressed while the editor is open
void editor_keypress(int key) {
  if (key == SDLK_X) {
    editor.mode = EDITOR_MODE_PORTAL;
  } else if (key == SDLK_F) {
    if (get_sector_of_point(state.mouse.pos) != NULL) {
      LineSegment *line = get_line_at_point(state.mouse.pos, get_sector_of_point(state.mouse.pos)->lines,
                                            get_sector_of_point(state.mouse.pos)->line_count, 1000);

      if (line != NULL && line->portal != NULL) {
        line->portal->flipped = !line->portal->flipped;
      }
    }
  } else if (key == SDLK_S && editor.point_count > 2) {
    create_sector_with_points();
    printf("created sector\n");
    editor.mode = EDITOR_MODE_SECTOR;
  } else if (key == SDLK_B) {
    editor.mode = EDITOR_MODE_STARTPOS;
  } else if (key == SDLK_L) {
    editor.mode = EDITOR_MODE_LINE;
    editor.point_count = 0;
  } else if (key == SDLK_ESCAPE) { // reset to start
    editor.mode = EDITOR_MODE_SECTOR;
    editor.point_count = 0;
  }
}

/* --- DRAWING TEXT FUNCTIONS --- */
// text that appears that tells you what you're missing as well as helping you edit stuff.

void draw_editor_text() {
  if (editor.mode == EDITOR_MODE_PORTAL) {
    write_string("portal mode", (vec2f){WINDOW_WIDTH - (10 * 12) - 1, 11}, (rgba){255, 255, 255, 255}, 1);
  } else if (editor.mode == EDITOR_MODE_SECTOR) {
    write_string("map mode", (vec2f){WINDOW_WIDTH - (10 * 9) - 1, 11}, (rgba){255, 255, 255, 255}, 1);
  } else if (editor.mode == EDITOR_MODE_PLACING_POINTS) {
    write_string("creating sector", (vec2f){WINDOW_WIDTH - (10 * 15) - 1, 11}, (rgba){255, 255, 255, 255}, 1);
  }

  if (editor.mode != EDITOR_MODE_SECTOR) {
    write_string("press esc to stop", (vec2f){WINDOW_WIDTH - (10 * 17) - 1, 22}, (rgba){255, 255, 255, 255}, 1);
  }

  if (state.start.pos.x == 0 && state.start.pos.y == 0) {
    write_string("your map is missing:\na start position", (vec2f){10, WINDOW_HEIGHT - (11 * 4)},
                 (rgba){255, 64, 64, 255}, 1);
  }
}

// text that appears next to your mouse when hovering over a line
void draw_mouse_string(LineSegment line) {
  char data[100];
  sprintf(data, "sector id: %d\nline id: %d", line.sector_id, line.id);
  write_string(data, (vec2f){state.mouse.pos.x + 5, state.mouse.pos.y - 20}, (rgba){255, 255, 255, 255}, 1);
  if (editor.mode != EDITOR_MODE_PORTAL) {
    write_string("click to change line color", (vec2f){state.mouse.pos.x + 5, state.mouse.pos.y}, line.color, 1);
  } else {
    if (editor.point_count == 0) {
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

/* --- DRAWING FUNCTIONS --- */
// these functions display stuff in the editor, such as the sectors, the start pos, the player pos etc...
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

// Show the start position
void draw_start_pos() {
  // draw the start pos
  if (editor.mode == EDITOR_MODE_STARTPOS) {
    rgba color;
    if (get_sector_of_point(state.mouse.pos) == NULL) {
      color = (rgba){255, 0, 0, 255};

    } else {
      color = (rgba){255, 255, 0, 255};
    }
    write_character(state.mouse.pos, '+', color, 2);
  } else if (state.start.pos.x != 0.0 || state.start.pos.y != 0.0) {
    write_character(state.start.pos, '+', (rgba){255, 255, 0, 255}, 2);
  }
}

// Lines used to help the user connect portals or show where the line currently is
void draw_helper_lines() {
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
  } else if (editor.mode == EDITOR_MODE_PORTAL && editor.point_count > 0) {
    framebuf_line_s(&framebuf, editor.points_placed[0].x, editor.points_placed[0].y, state.mouse.pos.x,
                    state.mouse.pos.y, (rgba){128, 128, 128, 128});
  }
}

// draw the position of the player, yellow if in bounds, red if out of bounds.
void draw_player_pos() {
  rgba playercol = (rgba){0};
  if (get_sector_of_point(state.player.pos) == NULL) {
    playercol = (rgba){255, 0, 0, 255};
  } else {
    playercol = (rgba){255, 255, 0, 255};
  }
  vec2f drawpos = (vec2f){state.player.pos.x - 4, state.player.pos.y - 6};
  write_character(drawpos, 'p', playercol, 1);
}

void draw_raycasts() {
  for (int i = -1; i < WINDOW_WIDTH; i++) {
    float rot = state.camera.rot - state.camera.fov / 2.0 + (i / (float)WINDOW_WIDTH) * state.camera.fov;
    Sector *sec = get_sector_of_point(state.camera.pos);
    if (sec != NULL) {
      raycast_sec(sec, state.camera.pos, rot, state.camera.dist);
    }
  }
}

// Draws all the line segments in each sector for each sector
void draw_line_segments(int precision) {
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
}

// the main function - all functions to draw stuff are called here
void render_map() {
  // draw the raycast rays
  draw_raycasts();

  // draw the line segments
  draw_line_segments(MOUSE_TOLERANCE);

  // visuals to help with the editor
  draw_editor_text();
  draw_sector_creation();

  // Show the start position
  draw_start_pos();

  // Lines for connecting stuff or creating lines in the editor
  draw_helper_lines();

  // show the player posiiton
  draw_player_pos();
}

/*  --- MISC FUNCTIONS --- */

// initate the editor
void editor_init() {
  editor = (Editor){0};
  editor.map_mode = 1;
}

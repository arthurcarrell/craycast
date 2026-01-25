#include "editor.h"
#include "framebuf.h"
#include "line.h"
#include "sector.h"
#include "state.h"
#include "utils.h"

Editor editor;

void render_map() {
  // draw the raycast rays
  for (int i = -1; i < WINDOW_WIDTH; i++) {
    float rot = state.camera.rot - state.camera.fov / 2.0 +
                (i / (float)WINDOW_WIDTH) * state.camera.fov;
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
        if (line.flags & LINE_FLAG_PORTAL ||
            line.flags & LINE_FLAG_PORTAL_EXIT) {
          if (is_on_line(state.mouse.pos,
                         lineseg_line(sec->lines[line.portal->output_id]),
                         precision)) {
            final_color = (rgba){255, 136, 0, 255};
          }
        }
      } else {
        final_color = (rgba){0, 174, 255, 255};
      }
      framebuf_line_s(&framebuf, line.start.x, line.start.y, line.end.x,
                      line.end.y, final_color);

      if (line.flags & LINE_FLAG_PORTAL_EXIT) {
      }
    }
  }

  // draw a gray line to indicate a line in progress
}

void editor_on_click() {
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
    Sector *current =
        sector_create(5, (rgba){0, 0, 0, 255}, 20, (rgba){0, 0, 0, 255}, 0, 0);

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
                *portal = (Portal){line->id, line->sector_id, 0};
                prev->portal = portal;
              } else {
                prev->portal->output_id = line->id;
              }

              prev->flags |= LINE_FLAG_PORTAL;

              if (line->portal == NULL) {
                // create a portal struct
                Portal *portal = malloc(sizeof(Portal));
                *portal = (Portal){prev->portal->output_id, line->sector_id, 0};
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

void editor_init() {
  editor = (Editor){0};
  editor.even_click = 1;
  editor.map_mode = 1;
  editor.color = (rgba){255, 255, 255, 255};
  editor.last_click_pos = (vec2f){0, 0};
  editor.portal_mode = 0;
  editor.last_line_id = -1;
  editor.last_sector_id = 0;
  editor.current_sector = 0;
}

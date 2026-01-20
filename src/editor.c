#include "editor.h"
#include "framebuf.h"
#include "state.h"
#include "utils.h"

Editor editor;

void render_map() {
  // draw the raycast rays
  for (int i = -1; i < WINDOW_WIDTH; i++) {
    float rot = state.camera.rot - state.camera.fov / 2.0 +
                (i / (float)WINDOW_WIDTH) * state.camera.fov;
    raycast(state.camera.pos, rot, state.camera.dist);
  }

  // for each line, draw one
  int precision = 1000;
  rgba final_color = (rgba){0};
  for (int i = 0; i < state.line_count; i++) {
    Line line = state.lines[i];
    if (!is_on_line(state.mouse.pos, line, precision)) {
      final_color = line.color;
      if (line.flags & LINE_FLAG_PORTAL || line.flags & LINE_FLAG_PORTAL_EXIT) {
        if (is_on_line(state.mouse.pos, state.lines[line.portal.output_id],
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

  // draw a gray line to indicate a line in progress
  if (!editor.even_click) {
    framebuf_line_s(&framebuf, editor.last_click_pos.x, editor.last_click_pos.y,
                    state.mouse.pos.x, state.mouse.pos.y,
                    (rgba){118, 118, 118, 128});
  }
}

void editor_init() {
  editor.even_click = 1;
  editor.map_mode = 1;
  editor.color = (rgba){255, 255, 255, 255};
  editor.last_click_pos = (vec2f){0, 0};
  editor.portal_mode = 0;
  editor.last_line_id = -1;
}

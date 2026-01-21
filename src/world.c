
#include "framebuf.h"
#include "raycast.h"
#include "sector.h"
#include "state.h"
#include "utils.h"
#include <math.h>

void render_world() {
  // do a raycast for each pixel on the screen
  for (int i = 0; i < WINDOW_WIDTH; i++) {
    float rot = state.camera.rot - state.camera.fov / 2.0 +
                (i / (float)WINDOW_WIDTH) * state.camera.fov;
    Sector *sec = get_sector_of_point(state.camera.pos);
    if (sec != NULL) {
      Raycast result =
          raycast_sec(sec, state.camera.pos, rot, state.camera.dist);

      if (result.hit) {
        // correct distance to remove the fisheye
        result.distance = result.distance * cosf(rot - state.camera.rot);
      }
      state.camera.result[i] = result;
    }
  }

  // store the results in arrays to be handed to the framebuffer
  int top[WINDOW_WIDTH];
  int bottom[WINDOW_WIDTH];
  uint32_t wallcolor[WINDOW_WIDTH];

  int ceil_top[WINDOW_WIDTH];
  int ceil_bottom[WINDOW_WIDTH];
  int floor[WINDOW_WIDTH];
  uint32_t ceilcolor[WINDOW_WIDTH];
  uint32_t floorcolor[WINDOW_WIDTH];

  int tile_size = 20;
  float projection_dist = (WINDOW_WIDTH / 2.0) / tanf(state.camera.fov / 2.0);
  // run through each raycast and draw a line on the screen for it.
  for (int i = 0; i < WINDOW_WIDTH; i++) {
    Raycast result = state.camera.result[i];
    if (result.hit) {

      Sector *currsec = &state.sectors[result.sector_id];

      tile_size = currsec->ceil_height - currsec->floor_height;
      float height = (tile_size / result.distance) * projection_dist;
      int start = WINDOW_HEIGHT / 2.0 - height / 2;
      int end = WINDOW_HEIGHT / 2.0 + height / 2;
      rgba color = state.sectors[result.sector_id].lines[result.line_id].color;

      int darkness =
          clampf((result.distance / 1.5f) - currsec->light_modifer, 255, 0);

      int r = (color.r - darkness) > 0 ? (color.r - darkness) : 0;
      int g = (color.g - darkness) > 0 ? (color.g - darkness) : 0;
      int b = (color.b - darkness) > 0 ? (color.b - darkness) : 0;

      // store the results into an array so that the framebuffer doesnt have a
      // hard time
      top[i] = start > 0 ? start : 0;
      bottom[i] = end >= WINDOW_HEIGHT ? WINDOW_HEIGHT - 1 : end;
      wallcolor[i] = rgba_to_int((rgba){r, g, b, 255});

      // floor and ceiling
      ceilcolor[i] = rgba_to_int(currsec->ceil_color);
      floorcolor[i] = rgba_to_int(currsec->floor_color);
      ceil_top[i] = 0;
      ceil_bottom[i] = top[i] - 1;
    } else {
      top[i] = -1;
      bottom[i] = -1;
      wallcolor[i] = 0;
      ceil_top[i] = -1;
      ceil_bottom[i] = -1;
    }
  }
  // give the framebuffer the wall info
  framebuf_column_optimised(&framebuf, top, bottom, wallcolor, WINDOW_WIDTH);
  framebuf_column_optimised(&framebuf, ceil_top, ceil_bottom, ceilcolor,
                            WINDOW_WIDTH);
}

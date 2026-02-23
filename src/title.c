#include "font.h"
#include "state.h"
#include "utils.h"

void render_title() {
  write_string("c raycast experiment", (vec2f){WINDOW_WIDTH / 2, 30}, (rgba){255, 255, 255, 255}, 3);

  vec2f pos = (vec2f){10, WINDOW_HEIGHT / 2 - 30};
  write_string("select a map:", pos, (rgba){255, 255, 255, 255}, 1);
}

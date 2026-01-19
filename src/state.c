#include "state.h"

State state;

void state_init() {
  // these all need to be instantiated as otherwise C uses garbage values that
  // could literally be anything
  state.line_count = 0;

  state.camera.pos = (vec2f){50, WINDOW_HEIGHT / 2};
  state.camera.rot = 0.0;
  state.camera.fov = deg_to_radians(60);
  state.camera.dist =
      500; // darkness means that after ~300-400 nothing will show anyway

  state.mouse.pos = (vec2f){0, 0};

  state.delta = 0;
}

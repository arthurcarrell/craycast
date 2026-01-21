#include "state.h"
#include "line.h"
#include "sector.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
State state;

void state_init() {
  // Set all values in state to zero, set all pointers to NULL
  state = (State){0};

  // --- Lines ---
  state.lines = calloc(100, sizeof(LineSegment));
  if (state.lines == NULL) {
    fprintf(stderr, "Failed to allocate to state.lines\n");
    exit(1);
  }

  // --- Camera ---

  state.camera.pos = (vec2f){50, WINDOW_HEIGHT / 2};
  state.camera.rot = 0.0;
  state.camera.fov = deg_to_radians(60);
  state.camera.dist =
      500; // darkness means that after ~300-400 nothing will show anyway

  // --- MISC ---
  state.mouse.pos = (vec2f){0, 0};
  state.mouse.canary = 0xDECAFBAD; // used to check if data has been corrupted,
                                   // I was having a problem
  state.delta = 0;

  // --- Sectors ---
  state.sectors = calloc(100, sizeof(Sector));
  state.sector_count = 0;

  // for now, just create a sector
  // sector_create(20, (rgba){0, 0, 0, 255}, 255, (rgba){0, 0, 0, 255}, 0, 0);
  // done :)
  printf("State initalised!\n");
}

void state_destroy() {
  destroy_sectors(&state.sectors, &state.sector_count);
  destroy_linesegs(state.lines, &state.line_count);
}

#include "state.h"
#include "actor.h"
#include "line.h"
#include "sector.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
State state;

void state_init() {
  // Set all values in state to zero, set all pointers to NULL
  state = (State){0};
  // --- Camera ---

  state.camera.pos = (vec2f){50, WINDOW_HEIGHT / 2};
  state.camera.rot = 0.0;
  state.camera.fov = deg_to_radians(1);
  state.camera.dist = 400; // darkness means that after ~300-400 nothing will show anyway
  state.in_menu = 1;
  // --- MISC ---
  state.mouse.pos = (vec2f){0, 0};
  state.delta = 0;

  // -- ACTORS --
  state.player = (Actor){state.camera.pos, 0};
  state.end = (Actor){0};
  state.start = (Actor){0};

  // --- Sectors ---
  state.sectors = calloc(100, sizeof(Sector));
  state.sector_count = 0;

  printf("State initalised!\n");
}

void state_destroy() { destroy_sectors(&state.sectors, &state.sector_count); }

#include "actor.h"
#include "raycast.h"
#include "sector.h"
#include "utils.h"
#include <stdio.h>
#include <wchar.h>

int actor_move(Actor *actor, float direction, float amount) {
  // raycast forward that direction
  if (get_sector_of_point(actor->pos) != NULL) {
    Raycast ray = raycast_sec(get_sector_of_point(actor->pos), actor->pos,
                              direction, amount);

    // if raycast hasnt hit anything, move forward
    if (ray.hit == 0) {
      printf("moved\n");
      actor->pos = ray.pos;
      return 1;
    } else {
      return 0;
    }
  }
  return 0;
}

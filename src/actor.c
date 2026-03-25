#include "actor.h"
#include "raycast.h"
#include "sector.h"
#include <math.h>
#include <stdio.h>
#include <wchar.h>

int actor_move(Actor *actor, float amount, float direction) {
  // raycast forward that direction
  if (get_sector_of_point(actor->pos) != NULL) {
    Raycast ray = raycast_sec(get_sector_of_point(actor->pos), actor->pos, direction, amount);

    // if raycast hasnt hit anything, move forward
    if (!ray.hit) {
      actor->pos = ray.pos;
      actor->rot += (ray.rot - direction);

      // normalise
      actor->rot = fmodf(actor->rot, M_PI * 2);
      if (actor->rot < 0) {
        actor->rot += M_PI * 2;
      }
    }
  } else {
    printf("out of bounds! POS: (%f,%f)\n", actor->pos.x, actor->pos.y);
  }
  return 0;
}

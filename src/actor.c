#include "actor.h"
#include "raycast.h"
#include "sector.h"
#include <stdio.h>
#include <wchar.h>

int actor_move(Actor *actor, float amount, float direction) {
  // raycast forward that direction
  if (get_sector_of_point(actor->pos) != NULL) {
    Raycast ray = raycast_sec_with_skip(get_sector_of_point(actor->pos),
                                        actor->pos, direction, amount, 2.1);

    // if raycast hasnt hit anything, move forward
    if (ray.hit == 0) {
      // HACK: dont change the player rot unless gone through a portal
      if (get_sector_of_point(ray.pos) != NULL) {
        actor->pos = ray.pos;
        if (get_sector_of_point(ray.pos)->id !=
            get_sector_of_point(actor->pos)->id) {
          actor->rot = ray.rot;
        }
      }
      return 1;
    }
  } else {
    printf("out of bounds!\n");
  }
  return 0;
}

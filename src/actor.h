#ifndef ACTOR_H
#define ACTOR_H

#include "utils.h"
typedef struct {
  vec2f pos;
  float rot;
  int height;

} Actor;

int actor_move(Actor *actor, float amount, float direction);
#endif

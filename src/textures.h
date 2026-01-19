#ifndef TEXTURES_H
#define TEXTURES_H
#include <SDL3/SDL_render.h>

#define TEXTURE_FIT_NONE 0    // do nothing
#define TEXTURE_FIT_STRETCH 1 // stretch the texture to fit
#define TEXTURE_FIT_REPEAT 2  // repeat the texture
#define TEXTURE_FIT_FILL 3    // stretch the texture but keep the ratio

typedef struct {
  SDL_Texture *texture;
  int fit;
} Texture;

static struct {
  Texture textures[5];
  int count;
} Textures;

#endif // !TEXTURES_H

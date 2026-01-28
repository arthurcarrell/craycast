#include "textures.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <stdio.h>

static int load_textures(SDL_Renderer *renderer) { return 0; }

void load_texture(char *name) {
  char *path = NULL;
  // load the path into path:
  SDL_asprintf(&path, "%stextures/%s", SDL_GetBasePath(), name);
  SDL_Surface *surface = SDL_LoadPNG(path);
  if (surface != NULL) {
    Textures.textures[Textures.count++] = (Texture){surface, TEXTURE_FIT_FILL};
  } else {
    printf("Error loading surface! %s\n", SDL_GetError());
  }
  SDL_free(path);
}
int textures_init() { return 0; }

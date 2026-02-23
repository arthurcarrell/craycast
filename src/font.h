#ifndef FONT_H
#define FONT_H

#include "utils.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>

typedef struct {
  SDL_Surface *surface;
  int width;
  int height;
  char chars[49];
} Font;

extern Font font;

int font_init(SDL_Renderer *renderer);
void write_character(vec2f pos, char chr, rgba color, int size); // TODO: Dont expose this
void write_string(char *string, vec2f pos, rgba color, int size);
void font_destroy();
#endif

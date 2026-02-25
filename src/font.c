#include "font.h"
#include "framebuf.h"
#include "utils.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

Font font;
int load_font(char *path, SDL_Renderer *renderer) {
  font.surface = IMG_Load(path);

  if (font.surface == NULL) {
    fprintf(stderr, "Failed to load texture!");
    exit(1);
  }

  font.width = font.surface->w;
  font.height = font.surface->h;

  strcpy(font.chars, "0123456789abcdefghijklmnopqrstuvwxyz!?.\":/%()+-F");
  return 1;
}

void write_character(vec2f pos, char chr, rgba color, int size) {
  // get the location of the character in the font
  int index;
  int found = 0;
  for (index = 0; index < 49; index++) {
    if (chr == font.chars[index]) {
      // character found!
      found = 1;
      break;
    }
  }

  if (!found) {
    return;
  }

  int starty = index * 9;
  for (int y = 0; y < 9; y++) {
    for (int x = 0; x < 8; x++) {
      Uint8 a;
      SDL_ReadSurfacePixel(font.surface, x, starty + y, NULL, NULL, NULL, &a);

      if (a != 0) {
        for (int sx = 0; sx < size; sx++) {
          for (int sy = 0; sy < size; sy++) {
            framebuf_point_int(&framebuf, pos.x + x * size + sx, pos.y + y * size + sy, color);
          }
        }
      }
    }
  }
}

void write_string(char *string, vec2f pos, rgba color, int size) {
  int length = strlen(string);
  int originalx = pos.x;
  int gap = 1;
  for (int i = 0; i < length; i++) {
    if (string[i] != '\n') {
      write_character(pos, string[i], color, size);
      pos.x += gap + (9 * size);
    } else {
      pos.x = originalx;
      pos.y += 10 * size;
    }
  }
}

int get_text_width(int text_size, int size) { return 1 + 9 * size * text_size; }

int font_init(SDL_Renderer *renderer) {
  char *path = malloc((strlen("font") + 15) * sizeof(char));
  sprintf(path, "textures/%s.png", "font");

  int result = load_font(path, renderer);

  free(path);
  return result;
}

void font_destroy() {
  SDL_DestroySurface(font.surface);
  font.surface = NULL;
}

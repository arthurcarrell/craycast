#include "framebuf.h"
#include "state.h"
#include "utils.h"
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

// This took forever....

// --- Framebuffer ---
// This thing has almost no context of the game environment and only does one
// thing: rendering.
//
// Once initiated, lines can be drawn easily with framebuf_line();
// For higher performance: framebuf_line_s should be used as it doesnt have to
// pack a line
// Additionally: if you know what direction a line is going to be (e.g.
// horizontal, vertical, diagonal) before runtime (such as drawing vertical
// lines for first person rendering), using that function is better.
//
// Additionally, if a lot of vertical lines are being drawn, its much
// better to store all of the points and colors in an array and just give it to
// the framebuf at once with framebuf_column_optimised();

Framebuffer framebuf;

SDL_Renderer *framebuf_init(Framebuffer *fb, SDL_Window *window, rgba color) {
  // create pixels
  fb->pixels = malloc(WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
  if (fb->pixels == NULL) {
    fprintf(stderr, "Failed to allocate pixels in framebuffer!\n");
    exit(1);
  }
  fb->background = color;
  framebuf_clear(fb);

  // create the surface to render on
  fb->screen = SDL_GetWindowSurface(window);
  // check it didnt fail
  if (fb->screen == NULL) {
    // uh oh
    fprintf(stderr, "Failed to create surface! %s\n", SDL_GetError());
    exit(1);
  }

  fb->format = SDL_GetPixelFormatDetails(
      fb->screen->format); // clang is crying because ive set a non constant
                           // value to a constant value - cry about it clang

  // create  a renderer to go with it
  SDL_Renderer *renderer = SDL_CreateSoftwareRenderer(fb->screen);
  if (renderer == NULL) {
    fprintf(stderr, "Failed to create renderer! %s\n", SDL_GetError());
  }
  printf("Framebuffer initalised!\n");
  return renderer;
}

void framebuf_screen(Framebuffer *fb, SDL_Window *window) {
  SDL_LockSurface(fb->screen);

  uint8_t *dst = (uint8_t *)fb->screen->pixels;
  uint8_t *src = (uint8_t *)fb->pixels;
  int pitch = fb->screen->pitch;
  if (pitch == WINDOW_WIDTH * sizeof(uint32_t)) {
    memcpy(fb->screen->pixels, fb->pixels,
           WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
  } else {
    for (int y = 0; y < WINDOW_HEIGHT; y++) {
      memcpy(dst + y * pitch, src + y * WINDOW_WIDTH * sizeof(uint32_t),
             WINDOW_WIDTH * sizeof(uint32_t));
    }
  }

  SDL_UnlockSurface(fb->screen);
  SDL_UpdateWindowSurface(window);
}

void framebuf_destroy(Framebuffer *fb) {
  free(fb->pixels);
  SDL_DestroySurface(fb->screen);
}
void framebuf_clear(Framebuffer *fb) {
  // the biggest optimisation I've made, god bless memset
  // Changing from a for loop to this has instantly increased framerate by 30
  memset(fb->pixels, 0, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(uint32_t));
}

void framebuf_point(Framebuffer *fb, vec2i point, rgba color) {
  fb->pixels[point.x + point.y * WINDOW_WIDTH] = rgba_to_int(color);
}

static inline void framebuf_point_int(Framebuffer *fb, int x, int y,
                                      rgba color) {
  if (x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT)
    return;
  fb->pixels[x + y * WINDOW_WIDTH] = rgba_to_int(color);
}

void framebuf_line_hor(Framebuffer *fb, int x0, int x1, int y, rgba color) {
  if (y < 0 || y >= WINDOW_HEIGHT) {
    return;
  }
  if (x0 > x1) {
    int swp = x0;
    x0 = x1;
    x1 = swp;
  }

  if (x1 < 0 || x0 >= WINDOW_WIDTH)
    return;

  if (x0 < 0)
    x0 = 0;
  if (x1 >= WINDOW_WIDTH)
    x1 = WINDOW_WIDTH - 1;

  uint32_t intcol = rgba_to_int(color);
  uint32_t *row = fb->pixels + y * WINDOW_WIDTH + x0;

  for (int i = 0; i <= x1 - x0; i++) {
    row[i] = intcol;
  }
}

void framebuf_line_vert(Framebuffer *fb, int x, int y0, int y1, rgba color) {
  if (x < 0 || x >= WINDOW_WIDTH) // its off the screen
    return;

  if (y0 > y1) {
    int swp = y0;
    y0 = y1;
    y1 = swp;
  }

  if (y1 < 0 || y0 >= WINDOW_HEIGHT)
    return;

  int y_start = (y0 < 0) ? 0 : y0;
  int y_end = (y1 >= WINDOW_HEIGHT) ? WINDOW_HEIGHT - 1 : y1;
  uint32_t col = rgba_to_int(color);

  uint32_t *row = fb->pixels + y_start * WINDOW_WIDTH + x;

  for (int y = y_start; y <= y_end; y++) {
    *row = col;
    row += WINDOW_WIDTH;
  }
}

void framebuf_column_optimised(Framebuffer *fb, int *top, int *bottom,
                               uint32_t *colors, int count) {
  // draw the range of pixels for each column
  for (int x = 0; x < WINDOW_WIDTH; x++) {
    if (top[x] < 0 || bottom[x] < 0)
      continue; // out of bounds
    uint32_t *ptr = fb->pixels + top[x] * WINDOW_WIDTH + x;
    int height = bottom[x] - top[x] + 1;
    for (int y = 0; y < height; y++) {
      *ptr = colors[x];
      ptr += WINDOW_WIDTH;
    }
  }
}
void framebuf_line_diag(Framebuffer *fb, int x0, int x1, int y0, int y1,
                        rgba color) {

  // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases
  int dx = abs(x1 - x0);
  int sx = (x0 < x1) ? 1 : -1;
  int dy = -abs(y1 - y0);
  int sy = (y0 < y1) ? 1 : -1;
  int err = dx + dy;

  uint32_t col = rgba_to_int(color);

  // yes its a while true break, this is cursed
  // indeed it is - this was the cause of a really bad error that took me a few
  // hours to fix
  while (1) {
    if (x0 >= 0 && x0 < WINDOW_WIDTH && y0 >= 0 && y0 < WINDOW_HEIGHT) {
      fb->pixels[y0 * WINDOW_WIDTH + x0] = col;
    }
    int err2 = 2 * err;
    if (err2 >= dy) {
      if (x0 == x1) {
        break;
      }
      err = err + dy;
      x0 = x0 + sx;
    }
    if (err2 <= dx) {
      if (y0 == y1) {
        break;
      }
      err = err + dx;
      y0 = y0 + sy;
    }
  }
}
void framebuf_line_s(Framebuffer *fb, int x0, int y0, int x1, int y1,
                     rgba color) {
  if (y0 == y1) {
    framebuf_line_hor(fb, x0, x1, y0, color);
  } else if (x0 == x1) {
    framebuf_line_vert(fb, x0, y0, y1, color);
  } else {
    framebuf_line_diag(fb, x0, x1, y0, y1, color);
  }
}
void framebuf_line(Framebuffer *fb, Line line) {
  framebuf_line_s(fb, line.start.x, line.start.y, line.end.x, line.end.y,
                  line.color);
}

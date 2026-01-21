#ifndef FRAMEBUF_H
#define FRAMEBUF_H

#include "state.h"
#include "utils.h"
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>
#include <stdint.h>

typedef struct {
  uint32_t *pixels;
  rgba background;
  SDL_Surface *screen;
  SDL_PixelFormatDetails *format;
} Framebuffer;

extern Framebuffer framebuf;

// --- Init & Destroy ---
SDL_Renderer *framebuf_init(Framebuffer *fb, SDL_Window *window, rgba color);
void framebuf_destroy(Framebuffer *fb);

// --- Draw functions ---
void framebuf_clear(Framebuffer *fb);
static inline void framebuf_point_int(Framebuffer *fb, int x, int y,
                                      rgba color);
void framebuf_line(Framebuffer *fb, Line line);
void framebuf_line_s(Framebuffer *fb, int x0, int y0, int x1, int y1,
                     rgba color);

void framebuf_line_vert(Framebuffer *fb, int x, int y0, int y1, rgba color);
void framebuf_column_optimised(Framebuffer *fb, int *top, int *bottom,
                               uint32_t *colors, int count);
// void framebuf_rect(Framebuffer *fb, vec2i start, vec2i end);

// --- Render functions ---
void framebuf_texture(Framebuffer *fb, SDL_Renderer *renderer);
void framebuf_screen(Framebuffer *fb, SDL_Window *window);
#endif // !FRAMEBUF_H

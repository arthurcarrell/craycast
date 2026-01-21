#include "sdl.h"
#include "state.h"
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>
#include <stdio.h>
#include <stdlib.h>

void sdl_init() {
  state.quit = 0;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    // something broke
    printf("Failed to initalise SDL: %s\n", SDL_GetError());
    exit(1);
  }

  // window and renderer
  state.window = SDL_CreateWindow("craycast", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
  if (state.window == NULL) {
    fprintf(stderr, "Failed to initalise window! %s", SDL_GetError());
    exit(1);
  }

  printf("SDL initalised!\n");
}

void sdl_destroy() {
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}

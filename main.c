#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 480

static struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event *events;
  SDL_Texture *texture;
  bool quit;
} state;

void init_sdl() {
  state.quit = false;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    // something broke
    printf("Failed to initalise SDL: %s\n", SDL_GetError());
  }

  int *test = malloc(sizeof(int));
  *test = 3;
  printf("My test number which I allocated myself: %i\n", *test);
  free(test);
}

int main(int argc, char *argv[]) {
  init_sdl();
  return 0;
}

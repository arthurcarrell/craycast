#include "sdl.h"
#include "state.h"

void init_sdl() {
  state.quit = false;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    // something broke
    printf("Failed to initalise SDL: %s\n", SDL_GetError());
  }

  // window and renderer
  SDL_CreateWindowAndRenderer("craycast", WINDOW_WIDTH, WINDOW_HEIGHT, 0,
                              &state.window, &state.renderer);
}

void destroy_sdl() {
  SDL_DestroyRenderer(state.renderer);
  SDL_DestroyWindow(state.window);
  SDL_Quit();
}

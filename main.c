#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <assert.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "utils.h"

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

static struct {
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  Line lines[100]; // will the program segfault after 100 lines are made? yes.
                   // this is a prototype so thats fine for now
  int line_count;

  struct {
    vec2f pos;
    float rot;
    float fov;
    int dist;
    Raycast result[WINDOW_WIDTH];
  } camera;

  struct {
    vec2f pos;
  } mouse;

  bool quit;
  float delta;

  bool map_mode;
} state;

static struct {
  rgba color;
  bool even_click;
  vec2f last_click_pos;
  bool portal_mode;
  int last_line_id;
} editor;

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

// Main functions
void init() {
  // these all need to be instantiated as otherwise C uses garbage values that
  // could literally be anything
  state.line_count = 0;

  state.camera.pos = (vec2f){50, WINDOW_HEIGHT / 2};
  state.camera.rot = 0.0;
  state.camera.fov = M_PI / 3.0f;
  state.camera.dist =
      500; // darkness means that after ~300-400 nothing will show anyway

  state.mouse.pos = (vec2f){0, 0};

  state.delta = 0;
  state.map_mode = false;

  // editor
  editor.even_click = true;
  editor.color = (rgba){255, 255, 255, 255};
  editor.last_click_pos = (vec2f){0, 0};
  editor.portal_mode = false;
  editor.last_line_id = -1;
}

// raycast
Raycast raycast(vec2f pos, float rot, float distance) {
  // create a line
  vec2f end = add_direction(pos, rot, distance);
  Line ray = (Line){pos, end, {255, 0, 0, 255}};

  float closest = distance;
  vec2f closest_vector = end;
  int line_id = -1;
  int success = 0;

  // get each line and get the closest one that intersects with the ray
  for (int i = 0; i < state.line_count; i++) {
    // check if lines intersect
    int found;
    vec2f result = get_line_intersections(&ray, &state.lines[i], &found);
    if (found) {
      float current_distance = get_distance(pos, result);
      if (current_distance < closest) {
        closest_vector = result;
        closest = current_distance;
        line_id = state.lines[i].id;
        success = 1;
      }
    }
  }

  // THINKING WITH PORTALS
  if (success && state.lines[line_id].flags & LINE_FLAG_PORTAL) {
    // this is a portal, so raycast from the line on the otherside
    Line line = state.lines[line_id];
    Line output = state.lines[line.portal.output_id];

    // check that the other portal has the PORTAL_EXIT flag, if they dont, quit.
    if (!(output.flags & LINE_FLAG_PORTAL_EXIT)) {
      printf("Error! Line %d is used in a portal, but is not marked as a "
             "portal exit.\n",
             output.id);
      destroy_sdl();
      exit(1);
    }

    // get the percentage of how far across the line the point is
    float percent = get_line_percent(closest_vector, line);

    // exit and raypos is black magic I looked up
    vec2f exit = {output.end.x - output.start.x, output.end.y - output.start.y};
    vec2f raypos = {output.start.x + exit.x * percent,
                    output.start.y + exit.y * percent};

    // get the relative angle of the ray from the portal
    float relrot = rot - get_direction(line.start, line.end);

    // check if flipped - if yes, flip the relative rotation
    if (output.portal.flipped) {
      relrot = -relrot;
    }
    // calculate the final angle for the exit line
    float rayrot = get_direction(output.start, output.end) + relrot;

    // if in map mode draw a purple line to indicate that the POV is going
    // through a portal
    if (state.map_mode) {
      SDL_SetRenderDrawColor(state.renderer, 255, 0, 255, 255);
      SDL_RenderLine(state.renderer, pos.x, pos.y, closest_vector.x,
                     closest_vector.y);
    }

    // move the position slightly forward so that it doesnt collide with itself
    raypos = add_direction(raypos, rayrot, 1);

    // shoot a new ray and add the distance so that it is not reset on return
    Raycast newray = raycast(raypos, rayrot, distance - closest);

    if (newray.hit) {
      newray.distance += closest;
    }
    return newray;

  } else if (success) {
    // draw a green line to represent success
    if (state.map_mode) {
      SDL_SetRenderDrawColor(state.renderer, 0, 255, 0, 255);
      SDL_RenderLine(state.renderer, pos.x, pos.y, closest_vector.x,
                     closest_vector.y);
    }
    return (Raycast){1, closest_vector, closest, line_id};
  }
  if (state.map_mode) {
    // draw a red line to indicate failure
    SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);
    SDL_RenderLine(state.renderer, ray.start.x, ray.start.y, ray.end.x,
                   ray.end.y);
  }

  return (Raycast){.hit = 0};
}
void update() {}

void render_map() {
  // draw the raycast rays
  for (int i = -1; i < WINDOW_WIDTH; i++) {
    float rot = state.camera.rot - state.camera.fov / 2.0 +
                (i / (float)WINDOW_WIDTH) * state.camera.fov;
    raycast(state.camera.pos, rot, state.camera.dist);
  }

  // for each line, draw one
  int precision = 1000;
  for (int i = 0; i < state.line_count; i++) {
    Line line = state.lines[i];
    if (!is_on_line(state.mouse.pos, line, precision)) {
      SDL_SetRenderDrawColor(state.renderer, line.color.r, line.color.g,
                             line.color.b, line.color.a);

      if (line.flags & LINE_FLAG_PORTAL || line.flags & LINE_FLAG_PORTAL_EXIT) {
        if (is_on_line(state.mouse.pos, state.lines[line.portal.output_id],
                       precision)) {
          SDL_SetRenderDrawColor(state.renderer, 255, 136, 0, 255);
        }
      }
    } else {
      SDL_SetRenderDrawColor(state.renderer, 0, 174, 255, 255);
    }
    SDL_RenderLine(state.renderer, line.start.x, line.start.y, line.end.x,
                   line.end.y);

    if (line.flags & LINE_FLAG_PORTAL_EXIT) {
    }
  }

  // draw a gray line to indicate a line in progress
  if (!editor.even_click) {
    SDL_SetRenderDrawColor(state.renderer, 119, 118, 123, 128);
    SDL_RenderLine(state.renderer, editor.last_click_pos.x,
                   editor.last_click_pos.y, state.mouse.pos.x,
                   state.mouse.pos.y);
  }
}

void render_world() {
  // do a raycast for each pixel on the screen
  for (int i = 0; i < WINDOW_WIDTH; i++) {
    float rot = state.camera.rot - state.camera.fov / 2.0 +
                (i / (float)WINDOW_WIDTH) * state.camera.fov;
    Raycast result = raycast(state.camera.pos, rot, state.camera.dist);
    if (result.hit) {
      // correct distance to remove the fisheye
      result.distance = result.distance * cosf(rot - state.camera.rot);
    }
    state.camera.result[i] = result;
  }

  int tile_size = 20;
  float projection_dist = (WINDOW_WIDTH / 2.0) / tanf(state.camera.fov / 2.0);
  // run through each raycast and draw a line on the screen for it.
  for (int i = 0; i < WINDOW_WIDTH; i++) {
    Raycast result = state.camera.result[i];
    float height = (tile_size / result.distance) * projection_dist;

    int start = WINDOW_HEIGHT / 2.0 - height / 2;
    int end = WINDOW_HEIGHT / 2.0 + height / 2;

    rgba color = state.lines[result.line_id].color;

    int darkness = clampf(result.distance / 1.5, 255, 0);

    int r = clamp(color.r - darkness, 255, 0);
    int g = clamp(color.g - darkness, 255, 0);
    int b = clamp(color.b - darkness, 255, 0);

    SDL_SetRenderDrawColor(state.renderer, r, g, b, color.a);
    SDL_RenderLine(state.renderer, i, start, i, end);
  }
}

void render() {
  if (state.map_mode) {
    render_map();
  } else {
    render_world();
  }
}

// Ran every time a mouse button is pressed
void event_mouse_down() {
  if (state.map_mode) {
    if (!editor.portal_mode) {
      if (!editor.even_click) {
        Line line = create_render_line(editor.last_click_pos, state.mouse.pos,
                                       editor.color);
        line.id = state.line_count;
        state.lines[state.line_count++] = line;
      }

      editor.last_click_pos = state.mouse.pos;
    } else if (state.map_mode && editor.portal_mode) {
      for (int i = 0; i < state.line_count; i++) {
        Line *line = &state.lines[i];
        if (is_on_line(state.mouse.pos, *line, 1000)) {
          if (editor.even_click) {
            editor.last_line_id = line->id;
            editor.last_click_pos = line->end;
          } else {
            // turn the previous line into a portal and the current line into a
            // portal exit
            Line *prev = &state.lines[editor.last_line_id];

            if (prev != line) {
              // check if changing portal target
              if (prev->flags & LINE_FLAG_PORTAL) {
                // yes, so strip the portal output flag from the portal output
                state.lines[prev->portal.output_id].flags &=
                    ~LINE_FLAG_PORTAL_EXIT;

                // will be overwritten but just in case
                prev->portal.output_id = 0;
              }
              prev->portal.output_id = line->id;
              prev->flags |= LINE_FLAG_PORTAL;

              line->flags |= LINE_FLAG_PORTAL_EXIT;
              line->portal.output_id = prev->id;
            }
            // disable portal mode
            editor.portal_mode = false;
            editor.last_click_pos = state.mouse.pos;
          }
        }
      }
    }
    editor.even_click = !editor.even_click;
  }
}

// Keyboard input
void get_keyboard_input() {
  // get the key pressed
  const bool *key_states = SDL_GetKeyboardState(0);

  float movespeed = 0.25 / 2;
  float look_sensitivity = 0.25;
  if (key_states[SDL_SCANCODE_LEFT]) {
    state.camera.rot -= 0.01745329 * state.delta * look_sensitivity;
  }
  if (key_states[SDL_SCANCODE_RIGHT]) {
    state.camera.rot += 0.01745329 * state.delta * look_sensitivity;
  }
  if (key_states[SDL_SCANCODE_W]) {
    state.camera.pos = add_direction(state.camera.pos, state.camera.rot,
                                     movespeed * state.delta);
  }
  if (key_states[SDL_SCANCODE_S]) {
    state.camera.pos = add_direction(state.camera.pos, state.camera.rot,
                                     -(movespeed * state.delta));
  }
  if (key_states[SDL_SCANCODE_A]) {
    state.camera.pos =
        add_direction(state.camera.pos, state.camera.rot + NINETY_DEGINRAD,
                      -(movespeed * state.delta));
  }
  if (key_states[SDL_SCANCODE_D]) {
    state.camera.pos =
        add_direction(state.camera.pos, state.camera.rot - NINETY_DEGINRAD,
                      -(movespeed * state.delta));
  }
}

void event_key_down(int key) {
  if (state.map_mode) {
    if (key == SDLK_W) {
      editor.color = (rgba){255, 255, 255, 255};
    } else if (key == SDLK_R) {
      editor.color = (rgba){255, 0, 0, 255};
    } else if (key == SDLK_Y) {
      editor.color = (rgba){255, 255, 0, 255};
    } else if (key == SDLK_G) {
      editor.color = (rgba){0, 255, 0, 255};
    } else if (key == SDLK_T) {
      editor.color = (rgba){0, 255, 255, 255};
    } else if (key == SDLK_B) {
      editor.color = (rgba){0, 0, 255, 255};
    } else if (key == SDLK_P) {
      editor.color = (rgba){255, 0, 255, 255};
    } else if (key == SDLK_X) {
      editor.portal_mode = !editor.portal_mode;
      editor.even_click = true;
    } else if (key == SDLK_F && editor.last_line_id != -1) {
      if (state.lines[editor.last_line_id].flags & LINE_FLAG_PORTAL_EXIT) {
        state.lines[editor.last_line_id].portal.flipped =
            !state.lines[editor.last_line_id].portal.flipped;
      }
    }
  }

  if (key == SDLK_Z) {
    state.map_mode = !state.map_mode;
    editor.portal_mode = false;
  }
}

// Where the program actually runs, keeps going until quit is true
int main(int argc, char *argv[]) {
  init_sdl();
  init();
  SDL_Event event;
  while (!state.quit) {

    // frame begins
    float start = SDL_GetTicks();

    // input comes before the events
    SDL_GetMouseState(&state.mouse.pos.x, &state.mouse.pos.y);

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_EVENT_QUIT:
        state.quit = true;
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
        event_mouse_down();
        break;
      case SDL_EVENT_KEY_DOWN:
        event_key_down(event.key.key);
      }
    }
    SDL_SetRenderDrawColor(state.renderer, 0, 0, 0, 255);
    SDL_RenderClear(state.renderer);
    get_keyboard_input();
    update();
    render();
    SDL_RenderPresent(state.renderer);
    SDL_Delay(
        10); // bandaid fix, I should implement an actual FPS cap but I was
             // literally hearing whistling while this used 100% of my CPU
    // frame ends
    state.delta = SDL_GetTicks() - start;
  }

  // The program has now ended, destroy SDL
  destroy_sdl();
  return 0;
}

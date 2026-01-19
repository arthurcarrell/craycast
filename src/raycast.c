#include "raycast.h"
#include "editor.h"
#include "sdl.h"
#include "state.h"
#include "utils.h"

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
    if (editor.map_mode) {
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
    if (editor.map_mode) {
      SDL_SetRenderDrawColor(state.renderer, 0, 255, 0, 255);
      SDL_RenderLine(state.renderer, pos.x, pos.y, closest_vector.x,
                     closest_vector.y);
    }
    return (Raycast){1, closest_vector, closest, line_id};
  }
  if (editor.map_mode) {
    // draw a red line to indicate failure
    SDL_SetRenderDrawColor(state.renderer, 255, 0, 0, 255);
    SDL_RenderLine(state.renderer, ray.start.x, ray.start.y, ray.end.x,
                   ray.end.y);
  }

  return (Raycast){.hit = 0};
}

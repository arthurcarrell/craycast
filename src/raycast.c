#include "raycast.h"
#include "editor.h"
#include "framebuf.h"
#include "line.h"
#include "state.h"
#include "utils.h"
#include <math.h>
#include <stdio.h>
#include <wchar.h>

int get_closest_intersection(Sector *sec, Line ray, LineSegment *exempt_line, float distance, int *success,
                             vec2f *closest_vector) {
  float closest = distance;
  int line_id = -1;
  *success = 0;
  // get each line and get the closest one that intersects with the ray
  for (int i = 0; i < sec->line_count; i++) {
    // check that the line is not the same as the exempt line
    if (exempt_line != NULL && sec->id == exempt_line->sector_id && sec->lines[i].id == exempt_line->id) {
      continue;
    }
    // check if lines intersect
    int found;
    Line line2 = lineseg_line(sec->lines[i]);
    vec2f result = get_line_intersections(&ray, &line2, &found);
    if (found) {
      float current_distance = get_distance(ray.start, result);
      if (current_distance < closest) {
        *closest_vector = result;
        closest = current_distance;
        line_id = sec->lines[i].id;
        *success = 1;
      }
    }
  }
  return line_id;
}

// Only raycast for a specific sector, faster as less lines have to be checked
Raycast raycast_sec_skip_line(Sector *sec, vec2f pos, float rot, float distance, int touched_portal,
                              LineSegment *exempt_line) {
  // create a line
  vec2f end = add_direction(pos, rot, distance);
  Line ray = (Line){pos, end, {255, 0, 0, 255}};

  int success;
  vec2f closest_vector = ray.end;
  int line_id = get_closest_intersection(sec, ray, exempt_line, distance, &success, &closest_vector);
  float closest = get_distance(ray.start, closest_vector);

  // THINKING WITH PORTALS
  if (success && sec->lines[line_id].flags & LINE_FLAG_PORTAL) {
    // this is a portal, so raycast from the line on the otherside
    LineSegment line = sec->lines[line_id];
    LineSegment output = state.sectors[line.portal->output_sector_id].lines[line.portal->output_id];

    // check that the other portal has the PORTAL_EXIT flag, if they dont, quit.
    if (!(output.flags & LINE_FLAG_PORTAL_EXIT)) {
      printf("Error! Line %d is used in a portal, but is not marked as a "
             "portal exit.\n",
             output.id);
      exit(1);
    }

    float anglein = get_direction(line.start, line.end);
    float angleout = get_direction(output.start, output.end);

    // calucate the relative angle of the ray to the portal
    float diff = angleout - anglein + M_PI;
    float offset = (output.portal->flipped) ? 0.0 : M_PI;

    // get the percentage of how far across the line the point is
    float percent = get_line_percent(closest_vector, lineseg_line(line));
    percent = (output.portal->flipped) ? (1.0 - percent) : percent;

    float rayrot = rot + diff + offset;

    vec2f exit = {output.end.x - output.start.x, output.end.y - output.start.y};
    vec2f raypos = {output.start.x + exit.x * percent, output.start.y + exit.y * percent};

    // normalise the angle
    rayrot = fmodf(rayrot, M_PI * 2);

    // if in map mode draw a purple line to indicate that the POV is going
    // through a portal
    if (editor.map_mode) {
      framebuf_line_s(&framebuf, pos.x, pos.y, closest_vector.x, closest_vector.y, (rgba){255, 0, 255, 255});
    }

    // add a small amount of distance
    raypos = add_direction(raypos, rayrot, 0.01f);

    // shoot a new ray and add the distance so that it is not reset on return
    // now has a line to ignore in the raycast - which means raycasts from portals cannot collide with themselves
    Raycast newray =
        raycast_sec_skip_line(&state.sectors[output.sector_id], raypos, rayrot, distance - closest, 1, exempt_line);

    if (newray.hit) {
      newray.distance += closest;
    }
    return newray;

  } else if (success) {
    // draw a green line to represent success
    if (editor.map_mode) {
      framebuf_line_s(&framebuf, pos.x, pos.y, closest_vector.x, closest_vector.y, (rgba){0, 255, 0, 255});
    }
    return (Raycast){1, touched_portal, closest_vector, closest, rot, line_id, sec->id};
  }
  if (editor.map_mode) {
    // draw a red line to indicate failure
    framebuf_line_s(&framebuf, ray.start.x, ray.start.y, ray.end.x, ray.end.y, (rgba){255, 0, 0, 255});
  }

  return (Raycast){.hit = 0, .hit_portal = touched_portal, {ray.end.x, ray.end.y}, closest, rot, -1, -1};
}

Raycast raycast_sec(Sector *sector, vec2f pos, float rot, float distance) {
  return raycast_sec_skip_line(sector, pos, rot, distance, 0, NULL);
}

// Raycast, getting the first LineSegment hit on any sector
Raycast raycast(vec2f pos, float rot, float distance) {
  // uh this is broke
  return raycast_sec(&state.sectors[0], pos, rot, distance);
}

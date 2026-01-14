#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>

#define DEFINE_DYNARR_STRUCT(type)                                             \
  typedef struct {                                                             \
    int size;                                                                  \
    type *value;                                                               \
    int length;                                                                \
  } type##DynArray

#define DEFINE_DYNARR_INIT(type)                                               \
  type##DynArray type##DynArray_initalise(type set_size) {                     \
    type##DynArray dynarr = (type##DynArray){};                                \
    dynarr.size = set_size;                                                    \
    dynarr.value = malloc(set_size * sizeof(type));                            \
    dynarr.length = 0;                                                         \
    if (dynarr.value == NULL) {                                                \
      printf("Could not initalise dynamic array!\n");                          \
      return (type##DynArray){.size = 0, .value = NULL, .length = 0};          \
    }                                                                          \
    return dynarr;                                                             \
  }

#define DEFINE_DYNARR_PUSH(type)                                               \
  void type##DynArray_push(type##DynArray *dynamicArray, type value) {         \
    if (dynamicArray->length >= dynamicArray->size) {                          \
      dynamicArray->size =                                                     \
          (dynamicArray->size == 0) ? 1 : dynamicArray->size * 2;              \
      int *newptr =                                                            \
          realloc(dynamicArray->value, dynamicArray->size * sizeof(type));     \
      if (newptr == NULL) {                                                    \
        printf("Could not add to dynamic array!\n");                           \
        return;                                                                \
      }                                                                        \
      dynamicArray->value = newptr;                                            \
    }                                                                          \
    dynamicArray->value[dynamicArray->length] = value;                         \
    dynamicArray->length++;                                                    \
  }

#define DEFINE_DYNAMIC_ARRAY(type)                                             \
  DEFINE_DYNARR_STRUCT(type);                                                  \
  DEFINE_DYNARR_INIT(type);                                                    \
  DEFINE_DYNARR_PUSH(type);

// Line flags
#define LINE_FLAG_PORTAL (1u << 1)

typedef struct {
  int x;
  int y;
} vec2;

typedef struct {
  float x;
  float y;
} vec2f;

typedef struct {
  int r;
  int g;
  int b;
  int a;
} rgba;

// Line:

typedef struct {
  vec2f start;
  vec2f end;
  rgba color;
  int id;
  unsigned int flags;
  struct {
    int output_id;
    float output_rot;
  } portal;
} Line;

typedef struct {
  int hit;
  vec2f pos;
  float distance;
  int line_id;
} Raycast;

vec2f get_line_intersections(const Line *line1, const Line *line2, int *found);
float get_distance(vec2f pos1, vec2f pos2);
vec2f add_direction(vec2f pos, float rot, float amount);
float clampf(float num, float max, float min);
int clamp(int num, int max, int min);

// lines
Line create_simple_line(vec2f start, vec2f end);
Line create_line_with_flags(vec2f start, vec2f end, unsigned int flags);
Line create_render_line(vec2f start, vec2f end, rgba color);
Line create_portal_line(vec2f start, vec2f end, int output_id,
                        float output_rot);

int is_on_line(vec2f pos, Line line, float precision);
float get_line_percent(vec2f pos, Line line);
// Dynamic Array, can be added to with realloc()
#endif // !UTILS_H

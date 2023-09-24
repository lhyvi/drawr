#ifndef BRUSH_C
#define BRUSH_C

#include "const.c"

#include "raylib.h"
#include <math.h>

enum BrushType {
  LINE,
  LINEBEZIERQUAD,
  LINECURL,
  CIRCLE,
};

static const char *DrawModeString[] = {"Line", "LineBezierQuad", "LineCurl",
                                       "Circle"};

enum SizeFlags {
  DISTANCE = 1,
  TIME = 2,
};

static const char *SizeFlagsString[] = {"Distance", "Time"};

enum ColorMode {
  BRUSH_COLOR,
  DATA_COLOR,
};

typedef struct Brush {
  char name[64];
  Color color;
  float size;
  enum BrushType type;
  int size_flags;
  enum ColorMode color_mode;
} Brush;

Brush Brush_get_default() {
  return (Brush){
      .name = "Default",
      .color = DRAWR_BLACK,
      .size = 2.0f,
      .type = LINE,
      .size_flags = 0,
      .color_mode = BRUSH_COLOR,
  };
}

Brush Brush_get_default_eraser() {
  return (Brush){
      .name = "Eraser",
      .color = DRAWR_WHITE,
      .size = 20.0f,
      .type = LINE,
      .size_flags = 0,
      .color_mode = BRUSH_COLOR,
  };
}

float Brush_get_size(const Brush *brush, double time) {
  float size = brush->size;
  if (brush->size_flags & TIME)
    size += (brush->size * (sin(time) + 1) / 2);
  return size;
}

Color Brush_get_color(const Brush *brush, Color data_color) {
  switch (brush->color_mode) {
  case BRUSH_COLOR:
    return brush->color;
  case DATA_COLOR:
    return data_color;
  }
}

#endif

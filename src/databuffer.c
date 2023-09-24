#ifndef DATABUFFER_C
#define DATABUFFER_C

#include "brush.c"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <stdlib.h>

#define BUFFER_MIN_CAP 16

#ifndef SCREENWIDTH
#define SCREENWIDTH 1280
#endif
#ifndef SCREENHEIGHT
#define SCREENHEIGHT 720
#endif

typedef struct {
  Vector2 vec;
  float time;
  Color color;
} Data;

typedef struct {
  int len;
  int size;
  float time;
  Data *data;
} DataBuffer;

DataBuffer DataBuffer_load() {
  DataBuffer new = {0};
  new.data = calloc(BUFFER_MIN_CAP, sizeof(Data));
  new.size = BUFFER_MIN_CAP;
  return new;
}

void DataBuffer_unload(DataBuffer *buffer) {
  free(buffer->data);
  *buffer = (DataBuffer){0};
}

void DataBuffer_extend(DataBuffer *buffer) {
  buffer->size *= 2;

  Data *new = realloc(buffer->data, buffer->size * sizeof(Data));

  if (new == NULL) {
    fprintf(stderr, "Couldn't reallocate data_buffer");
    abort();
  }
  buffer->data = new;
}

void DataBuffer_insert(DataBuffer *buffer, Data data) {
  if (buffer->len + 1 > buffer->size) {
    DataBuffer_extend(buffer);
  }

  buffer->data[buffer->len++] = data;
}

inline Data *DataBuffer_get(DataBuffer *buffer, int index) {
  if (index < 0 || index >= buffer->len)
    return NULL;
  return (buffer->data + index);
}

void DataBuffer_flip_y(DataBuffer *buffer, int height) {
  for (Data *data = buffer->data; data < buffer->data + buffer->len; ++data) {
    data->vec.y = height - data->vec.y;
  }
}

float DataBuffer_get_size(const DataBuffer *buffer, const Brush *brush,
                          const Data *start, const Data *end) {
  float size = Brush_get_size(brush, buffer->time);
  if (brush->size_flags & DISTANCE)
    size += Vector2Distance(start->vec, end->vec);
  if (brush->size_flags & TIME)
    size += brush->size * (sin(start->time + buffer->time) + 1) / 2;
  return size;
}

void DataBuffer_DrawLine(DataBuffer *buffer, const Brush *brush) {
  // if(buffer->len >= 4) {
  //   Vector2 points[buffer->len];
  //   for(int i = 0; i < buffer->len; ++i) {
  //     points[i] = buffer->data[i].vec;
  //   }
  //      DrawLineCatmullRom(points, buffer->len, 1.0f, BLACK);
  // }
  for (Data *start = buffer->data; start + 1 < buffer->data + buffer->len;
       start++) {
    Data *end = (start + 1);
    Color color = Brush_get_color(brush, start->color);

    DrawLineEx(start->vec, end->vec,
               DataBuffer_get_size(buffer, brush, start, end), color);
  }
}

void DataBuffer_DrawLineBezierQuad(DataBuffer *buffer, const Brush *brush) {
  for (Data *start = buffer->data; start + 2 < buffer->data + buffer->len;
       start++) {
    Data *end = (start + 2);
    Data *c1 = (start + 1);
    Color color = Brush_get_color(brush, c1->color);
    float size = DataBuffer_get_size(buffer, brush, start, end);

    DrawLineBezierQuad(start->vec, end->vec, c1->vec, size, color);
  }
}
void DataBuffer_DrawLineCurl(DataBuffer *buffer, const Brush *brush) {
  for (Data *start = buffer->data; start + 3 < buffer->data + buffer->len;
       start++) {
    Data *end = (start + 3);
    Data *c1 = (start + 1);
    Data *c2 = (start + 2);
    Color color = Brush_get_color(brush, c1->color);
    float size = DataBuffer_get_size(buffer, brush, start, end);

    DrawLineBezierCubic(c1->vec, c2->vec, start->vec, end->vec, size, color);
  }
}

void DataBuffer_DrawCircle(DataBuffer *buffer, const Brush *brush) {
  for (Data *start = buffer->data; start + 1 < buffer->data + buffer->len;
       start += 1) {
    Data *end = (start + 1);
    Color color = Brush_get_color(brush, start->color);
    float size = DataBuffer_get_size(buffer, brush, start, end) / 2;

    DrawCircleV(Vector2Lerp(start->vec, end->vec, 0.5), size, color);
  }
}

void DataBuffer_Draw(DataBuffer *buffer, const Brush *brush) {
  switch (brush->type) {
  case LINE:
    DataBuffer_DrawLine(buffer, brush);
    break;
  case LINEBEZIERQUAD:
    DataBuffer_DrawLineBezierQuad(buffer, brush);
    break;
  case LINECURL:
    DataBuffer_DrawLineCurl(buffer, brush);
    break;
  case CIRCLE:
    DataBuffer_DrawCircle(buffer, brush);
  }
}

void DataBuffer_clear(DataBuffer *buffer) {
  free(buffer->data);
  buffer->data = calloc(BUFFER_MIN_CAP, sizeof(Data));
  buffer->len = 0;
  buffer->size = BUFFER_MIN_CAP;
}

void DataBuffer_render_texture_dump(DataBuffer *buffer, const Brush *brush,
                                    RenderTexture render_texture,
                                    Vector2 texture_pos, float texture_scale) {

  if (buffer->len > 0)
    printf("Dumping data to texture (%d)\n", buffer->len);

  BeginTextureMode(render_texture);

  for (int i = 0; i < buffer->len; ++i) {
    /* Add texture_pos then scale by 1/texture_scale to match screen_buffer to
    render_texture size */
    buffer->data[i].vec = Vector2Scale(
        Vector2Subtract(buffer->data[i].vec, texture_pos), 1 / texture_scale);
    /* Screen buffer y starts at top_left
    Texture y starts at bottom_right
    Flip y to match */
    buffer->data[i].vec.y =
        render_texture.texture.height - buffer->data[i].vec.y;
  }

  DataBuffer_Draw(buffer, brush);

  EndTextureMode();

  DataBuffer_clear(buffer);
}

#endif

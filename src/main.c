#define RAYGUI_IMPLEMENTATION

#include "const.c"

#include "raygui.h"
#include "raylib.h"
#include "raymath.h"

#include "brush.c"
#include "canvas_operations.c"
#include "databuffer.c"
#include "texture_history.c"

#include <stdio.h>
#include <stdlib.h>

#define SCREENWIDTH 1280
#define SCREENHEIGHT 720

char *message = NULL;
float message_time = 0.0f;

void set_message(char *str) {
  if (message != NULL)
    free(message);
  message = str;
  message_time = 8.0f;
}

void show_raygui_icons() {
  for (int j = 0; j < 11; ++j)
    for (int i = 1; i < 20; ++i) {
      char *text = malloc(8);

      snprintf(text, 8, "#%d#", i + (j * 20));

      if (GuiButton((Rectangle){400 + (i * 32), 20 + (j * 32), 30, 30}, text)) {
        printf("Clicked %d\n", i + (j * 20));
      }
      free(text);
    }
}

void draw_cursor(double time, Color color, const Brush *brush, float scale) {
  DrawCircle(GetMouseX(), GetMouseY(), Brush_get_size(brush, time) / 2 * scale,
             Brush_get_color(brush, color));
  DrawCircleLines(GetMouseX(), GetMouseY(),
                  (Brush_get_size(brush, time) / 2 * scale) + 1, DRAWR_BLACK);
}

RenderTexture initialize_canvas() {
  RenderTexture canvas = LoadRenderTexture(SCREENWIDTH, SCREENHEIGHT);
  BeginTextureMode(canvas);
  SetTextureFilter(canvas.texture, TEXTURE_FILTER_TRILINEAR);
  ClearBackground(DRAWR_WHITE);
  EndTextureMode();
  return canvas;
}

int main(int argc, char *argv[]) {
  TextureHistory texture_history = {0};
  DataBuffer buffer = DataBuffer_load();

  Brush default_brush = Brush_get_default();
  Brush eraser = Brush_get_default_eraser();
  Brush rainbow_brush = {
      .name = "Rainbow",
      .color = DRAWR_WHITE,
      .size = 5.0f,
      .type = LINE,
      .size_flags = 0,
      .color_mode = DATA_COLOR,
  };
  Brush *curr_brush = &default_brush;

  InitWindow(SCREENWIDTH, SCREENHEIGHT, "Vin");
  GuiLoadStyleDefault();

  SetTargetFPS(120);
  SetWindowState(FLAG_WINDOW_RESIZABLE);
  SetGesturesEnabled(GESTURE_DRAG);

  double delta_time = 0;
  Color c = {0, 0, 0, 255};

  RenderTexture canvas = {0};
  if (argc > 1) {
    Texture image_texture = LoadTexture(argv[1]);
    if (image_texture.id != 0)
      RenderTexture_load_Texture(&canvas, image_texture);
    UnloadTexture(image_texture);
  }

  if (canvas.id == 0)
    canvas = initialize_canvas();

  bool show_canvas = true;
  bool show_debug = true;
  Vector2 canvas_pos = {.x = 0, .y = 0};
  float canvas_scale = 1;

  Vector2 drag_vector = {0};

  while (!WindowShouldClose()) {
    buffer.time = delta_time;
    c.r = ((float)(sin(delta_time + 0 * PI / 3) + 1) / 2) * 200 + 55;
    c.g = ((float)(sin(delta_time + 2 * PI / 3) + 1) / 2) * 200 + 55;
    c.b = ((float)(sin(delta_time + 4 * PI / 3) + 1) / 2) * 200 + 55;

    if (IsFileDropped()) {
      FilePathList droppedFiles = LoadDroppedFiles();
      printf("Dropped Files Count: %d\n", droppedFiles.count);
      for (int i = 0; i < droppedFiles.count; ++i) {
        bool loaded_image = false;
        printf("Dropped File: %s\n", droppedFiles.paths[i]);
        Texture texture = LoadTexture(droppedFiles.paths[i]);
        if (texture.id != 0) {
          RenderTexture_load_Texture(&canvas, texture);
          TextureHistory_clear(&texture_history);
          canvas_pos = (Vector2){0};
          canvas_scale = 1.0;

          loaded_image = true;
        }
        UnloadTexture(texture);
        if (loaded_image)
          break;
      }
      UnloadDroppedFiles(droppedFiles);
    }

    int char_pressed;
    while ((char_pressed = GetCharPressed()) != 0) {
      printf("%d %c\n", char_pressed, char_pressed);
      if (char_pressed == '!') {
        curr_brush->size_flags = 0;
      }
      if (char_pressed == '@') {
        curr_brush->size_flags ^= DISTANCE;
      }
      if (char_pressed == '#') {
        curr_brush->size_flags ^= TIME;
      }

      if (char_pressed - '1' == LINE) {
        curr_brush->type = LINE;
      }
      if (char_pressed - '1' == LINEBEZIERQUAD) {
        curr_brush->type = LINEBEZIERQUAD;
      }
      if (char_pressed - '1' == LINECURL) {
        curr_brush->type = LINECURL;
      }
      if (char_pressed - '1' == CIRCLE) {
        curr_brush->type = CIRCLE;
      }

      if (char_pressed == '[') {
        curr_brush->size = Clamp(curr_brush->size - 1, 1, INFINITY);
      }
      if (char_pressed == ']') {
        curr_brush->size += 1;
      }

      if (char_pressed == '-') {
        canvas_scale = Clamp(canvas_scale - 0.05, 0.1, 100);
      }
      if (char_pressed == '=') {
        canvas_scale = Clamp(canvas_scale + 0.05, 0.1, 100);
      }
    }

    int key_pressed;
    while ((key_pressed = GetKeyPressed())) {
      if (key_pressed == KEY_F1) {
        show_debug = !show_debug;
      }
      if (key_pressed == KEY_F2) {
        show_canvas = !show_canvas;
      }
      if (key_pressed == KEY_F3) {
        set_message(save_screen_to_file());
      }
      if (key_pressed == KEY_F4) {
        set_message(save_texture_to_file(canvas.texture));
      }
      if (key_pressed == KEY_F5) {
        BeginTextureMode(canvas);
        ClearBackground(DRAWR_WHITE);
        EndTextureMode();
        TextureHistory_clear(&texture_history);
        canvas_pos = (Vector2){0};
        canvas_scale = 1.0;
      }

      if (key_pressed == KEY_F6) {
        Image img = LoadImageFromTexture(canvas.texture);
        Image_sort_x(&img, canvas_scale, false);
        set_message(save_image_to_file(&img));
        UnloadImage(img);
      }

      if (key_pressed == KEY_F7) {
        Image img = LoadImageFromTexture(canvas.texture);
        Image_sort_y(&img, canvas_scale, false);
        set_message(save_image_to_file(&img));
        UnloadImage(img);
      }

      if (key_pressed == KEY_F8) {
        Image img = LoadImageFromTexture(canvas.texture);
        Image_sort_x(&img, canvas_scale, true);
        set_message(save_image_to_file(&img));
        UnloadImage(img);
      }

      if (key_pressed == KEY_F9) {
        Image img = LoadImageFromTexture(canvas.texture);
        Image_sort_y(&img, canvas_scale, true);
        set_message(save_image_to_file(&img));
        UnloadImage(img);
      }

      if (key_pressed == KEY_B) {
        curr_brush = &default_brush;
      }

      if (key_pressed == KEY_E) {
        curr_brush = &eraser;
      }

      if (key_pressed == KEY_N) {
        curr_brush = &rainbow_brush;
      }

      if (key_pressed == KEY_M) {
        TextureHistory_push_texture(&texture_history, canvas.texture);
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
          printf("Y\n");
          RenderTexture_flip_y(&canvas);
        } else {
          printf("X\n");
          RenderTexture_flip_x(&canvas);
        }
      }

      if (key_pressed == KEY_ZERO) {
        canvas_pos = (Vector2){0};
      }

      if (IsKeyDown(KEY_LEFT_CONTROL) && key_pressed == KEY_Z) {
        Image image = TextureHistory_pop(&texture_history);
        if (image.data != NULL) {
          Texture texture = LoadTextureFromImage(image);
          BeginTextureMode(canvas);
          ClearBackground(BLANK);
          DrawTexture(texture, 0, 0, WHITE);
          EndTextureMode();
          UnloadImage(image);
          UnloadTexture(texture);
          RenderTexture_flip_y(&canvas);
        }
      }
    }

    if (IsGestureDetected(GESTURE_DRAG) && IsKeyDown(KEY_SPACE)) {
      drag_vector =
          Vector2Multiply(GetGestureDragVector(),
                          (Vector2){GetScreenWidth(), GetScreenHeight()});
    } else {
      if (drag_vector.x != 0 || drag_vector.y != 0) {
        canvas_pos = Vector2Add(canvas_pos, drag_vector);
      }
      drag_vector = (Vector2){0};
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_SPACE)) {
      Data data = {
          .vec =
              {
                  .x = GetMouseX(),
                  .y = GetMouseY(),
              },
          .time = delta_time,
          .color = c,
      };
      if (buffer.len == 0) {
        DataBuffer_insert(&buffer, data);
        DataBuffer_insert(&buffer, data);
      }
      DataBuffer_insert(&buffer, data);
    } else {
      if (buffer.len != 0) {
        TextureHistory_push_texture(&texture_history, canvas.texture);
        DataBuffer_render_texture_dump(&buffer, curr_brush, canvas, canvas_pos,
                                       canvas_scale);
      }
    }

    BeginDrawing();

    ClearBackground(GRAY);

    if (show_canvas)
      DrawTextureEx(canvas.texture, Vector2Add(canvas_pos, drag_vector), 0,
                    canvas_scale, WHITE);
    float old_size = curr_brush->size;
    curr_brush->size *= canvas_scale;
    DataBuffer_Draw(&buffer, curr_brush);
    curr_brush->size = old_size;

    if (show_debug) {
      DrawFPS(0, 0);
      char cap_text[64];
      sprintf(cap_text, "History: %d\nBrush: %s\nDraw: %s\nSize: %1.0f",
              texture_history.len, curr_brush->name,
              DrawModeString[curr_brush->type], curr_brush->size);
      DrawText(cap_text, 0, 20, 20, c);
    }

    if (message != NULL)
      DrawText(message, 0, GetScreenHeight() - 20, 20, c);

    // showRayguiIcons();

    draw_cursor(delta_time, c, curr_brush, canvas_scale);

    EndDrawing();

    delta_time += GetFrameTime() * 10;

    if (message_time <= 0.0f) {
      if (message != NULL) {
        free(message);
        message = NULL;
      }
    } else {
      message_time -= GetFrameTime();
    }
  }

  DataBuffer_unload(&buffer);
  CloseWindow();

  return 0;
}

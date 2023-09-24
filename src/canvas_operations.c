#ifndef CANVAS_OPS_C
#define CANVAS_OPS_C

#include "const.c"

#include "raylib.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void RenderTexture_flip_y(RenderTexture *texture) {
  RenderTexture new =
      LoadRenderTexture(texture->texture.width, texture->texture.height);

  BeginTextureMode(new);
  ClearBackground(BLANK);
  DrawTexturePro(
      texture->texture,
      (Rectangle){0, 0, texture->texture.width, texture->texture.height},
      (Rectangle){0, 0, texture->texture.width, texture->texture.height},
      (Vector2){0, 0}, 0, WHITE);
  EndTextureMode();

  UnloadRenderTexture(*texture);
  *texture = new;
}

void RenderTexture_flip_x(RenderTexture *texture) {

  RenderTexture new =
      LoadRenderTexture(texture->texture.width, texture->texture.height);

  BeginTextureMode(new);
  ClearBackground(BLANK);
  DrawTexturePro(
      texture->texture,
      (Rectangle){0, 0, -texture->texture.width, -texture->texture.height},
      (Rectangle){0, 0, texture->texture.width, texture->texture.height},
      (Vector2){0, 0}, 0, WHITE);
  EndTextureMode();

  UnloadRenderTexture(*texture);
  *texture = new;
}

float Color_to_lumin(Color color) {
  return (0.00117254901 * color.r) + (0.00230196078 * color.g) +
         (0.00044705882 * color.b);
}

void RenderTexture_load_Texture(RenderTexture *texture, Texture image_texture) {
  UnloadRenderTexture(*texture);
  *texture = LoadRenderTexture(image_texture.width, image_texture.height);

  BeginTextureMode(*texture);
  ClearBackground(DRAWR_WHITE);
  DrawTexture(image_texture, 0, 0, WHITE);
  EndTextureMode();

  RenderTexture_flip_y(texture);
  printf("CANVAS ID : %d\n", texture->id);
}

int color_cmp(const void *a, const void *b) {
  Color ac = *((Color *)a);
  Color bc = *((Color *)b);
  float al = 0.299 * ac.r + 0.587 * ac.g + 0.114 * ac.b;
  float bl = 0.299 * bc.r + 0.587 * bc.g + 0.114 * bc.b;
  return ceil(al - bl);
}

int color_rev_cmp(const void *a, const void *b) {
  Color ac = *((Color *)a);
  Color bc = *((Color *)b);
  float al = 0.299 * ac.r + 0.587 * ac.g + 0.114 * ac.b;
  float bl = 0.299 * bc.r + 0.587 * bc.g + 0.114 * bc.b;
  return ceil(bl - al);
}

void Image_sort_x(Image *img, float threshold, bool reverse) {
  float *lumins;
  lumins = malloc(img->height * img->width * sizeof(float));
  for (int i = 0; i < img->height; i++) {
    for (int j = 0; j < img->width; j++) {
      int pixel_index = i * img->width + j;
      Color color = *((Color *)(img->data + (pixel_index * sizeof(Color))));
      *(lumins + pixel_index) = Color_to_lumin(color);
    }
  }

  for (int i = 0; i < img->height; i++) {
    for (int j = 0; j < img->width; j++) {
      int pixel_start = i * img->width + j;
      float curr = *(lumins + pixel_start);
      if ((threshold <= curr)) {
        int pixel_end = i * img->width + j;
        while ((threshold <= curr) && j + 1 < img->width) {
          pixel_end++;
          j++;
          curr = *(lumins + pixel_end);
        }
        qsort(img->data + (pixel_start * sizeof(Color)),
              pixel_end - pixel_start, sizeof(Color),
              reverse ? color_rev_cmp : color_cmp);
      }
    }
  }
}

void Image_sort_y(Image *img, float threshold, bool reverse) {
  ImageRotateCCW(img);
  Image_sort_x(img, threshold, reverse);
  ImageRotateCW(img);
}

char *save_image_to_file(Image *img) {
  time_t t = time(0);
  struct tm tm;
  localtime_s(&tm, &t);
  char *output_text = malloc(256);
  snprintf(output_text, 256, "output-%d-%02d-%02d %02d-%02d-%02d.png",
           tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
           tm.tm_sec);
  if (ExportImage(*img, output_text)) {
    return strcat(output_text, " - Successfully exported image");
  } else {
    return strdup("Image export failed.");
  }
}

char *save_screen_to_file() {
  Image img = LoadImageFromScreen();
  char *message = save_image_to_file(&img);
  UnloadImage(img);
  return message;
}

char *save_texture_to_file(Texture texture) {
  Image img = LoadImageFromTexture(texture);
  char *message = save_image_to_file(&img);
  UnloadImage(img);
  return message;
}

#endif

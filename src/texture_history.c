#ifndef HISTORY_C
#define HISTORY_C

#include "raylib.h"

#define HISTORY_CAP 32

typedef struct TextureHistory {
  Image images[HISTORY_CAP];
  int len;
} TextureHistory;

void TextureHistory_push(TextureHistory *history, Image image) {
  if (history->len == HISTORY_CAP) {
    UnloadImage(history->images[0]);
    for (int i = 1; i < HISTORY_CAP; ++i) {
      history->images[i - 1] = history->images[i];
    }
    history->len--;
  }
  history->images[history->len++] = image;
}

void TextureHistory_push_texture(TextureHistory *history, Texture texture) {
  Image image = LoadImageFromTexture(texture);
  TextureHistory_push(history, image);
}

void TextureHistory_clear(TextureHistory *history) {
  for (int i = 0; i < history->len; ++i) {
    UnloadImage(history->images[i]);
  }
  history->len = 0;
}

Image TextureHistory_pop(TextureHistory *history) {
  if (history->len <= 0) {
    return (Image){0};
  }
  return history->images[--history->len];
}

#endif

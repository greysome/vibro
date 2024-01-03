#include "gui.h"

void DrawShadowedText(const char *text, int x, int y, int font_size, Color color) {
  DrawText(text, x+4, y+2, font_size, BLACK);
  DrawText(text, x, y, font_size, color);
}

void DrawShadowedTextCenter(const char *text, int x, int y, int font_size, Color color) {
  Vector2 text_dimens = MeasureTextEx(GetFontDefault(), text, font_size, 0);
  x -= text_dimens.x / 2;
  y -= text_dimens.y / 2;
  DrawText(text, x+4, y+2, font_size, BLACK);
  DrawText(text, x, y, font_size, color);
}

void DrawShadowedTextSE(const char *text, int x, int y, int font_size, Color color) {
  Vector2 text_dimens = MeasureTextEx(GetFontDefault(), text, font_size, 0);
  DrawShadowedText(text, x-text_dimens.x, y-text_dimens.y, font_size, color);
}

int DrawAndMeasureShadowedText(const char *text, int x, int y, int font_size, Color color) {
  DrawShadowedText(text, x, y, font_size, color);
  return MeasureText(text, font_size);
}
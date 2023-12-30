#include "gui.h"

void DrawTextSE(char *text, int x, int y, int font_size, Color color) {
  Vector2 text_dimens = MeasureTextEx(GetFontDefault(), text, font_size, 0);
  DrawText(text, x - text_dimens.x, y - text_dimens.y, font_size, color);
}
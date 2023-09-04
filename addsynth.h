/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */

// I use _ADDSYNTH to prevent name conflict with a symbol in globals.h, used for
// representing the additive synthesis wave type
#ifndef _ADDSYNTH
#define _ADDSYNTH

#include "debug.h"
#include "globals.h"
#include "raylib.h"
#include "subgui.h"
#include "util.h"

SubGui subgui_addsynth;
bool show_addsynth = false;
bool addsynth_selecting = false;

void init_addsynth_subgui() {
  subgui_addsynth = (SubGui){
      .x = XMARGIN, .y = screenheight - 500, .w = 900, .h = 400, .m = 20};
}

void draw_addsynth_subgui() {
  draw_subgui_border(subgui_addsynth);

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
      within_subgui(GetMousePosition(), subgui_addsynth))
    addsynth_selecting = true;

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    addsynth_selecting = false;

  if (within_subgui(GetMousePosition(), subgui_addsynth)) {
    RelativeCoords c =
        absolute_to_relative(GetMousePosition(), subgui_addsynth);
    int j = c.x * NUM_HARMONICS;
    const char* s = TextFormat("%.04f", addsynth_coeffs[j]);
    Vector2 t = MeasureTextEx(GetFontDefault(), s, 15, 0.0);
    Vector2 v = relative_to_absolute(
        (RelativeCoords){(j + 0.5) / NUM_HARMONICS,
                         0.9 * (1 - addsynth_coeffs[j]) - 0.1},
        subgui_addsynth);
    DrawText(s, v.x - t.x / 2 + 3, v.y, 15, BLACK);
    DrawText(s, v.x - t.x / 2, v.y, 15, WHITE);
  }

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && addsynth_selecting) {
    RelativeCoords c =
        absolute_to_relative(GetMousePosition(), subgui_addsynth);
    int j = c.x * NUM_HARMONICS;
    if (0 <= j && j <= NUM_HARMONICS - 1)
      addsynth_coeffs[j] = clamp(1 - c.y / 0.9, 0, 1);
  }

  for (int i = 0; i < NUM_HARMONICS; i++) {
    Vector2 v = relative_to_absolute(
        (RelativeCoords){(i + 0.5) / NUM_HARMONICS, 1.0}, subgui_addsynth);
    const char* s = TextFormat("%d", i + 1);
    Vector2 t = MeasureTextEx(GetFontDefault(), s, 20, 0.0);
    DrawText(s, v.x - t.x / 2 + 3, v.y - t.y / 2, 20, BLACK);
    DrawText(s, v.x - t.x / 2, v.y - t.y / 2, 20, WHITE);

    v = relative_to_absolute((RelativeCoords){i / (float)NUM_HARMONICS,
                                              0.9 * (1 - addsynth_coeffs[i])},
                             subgui_addsynth);
    Vector2 w = relative_to_absolute(
        (RelativeCoords){(i + 1) / (float)NUM_HARMONICS, 0.9}, subgui_addsynth);
    DrawRectangle(v.x, v.y, w.x - v.x, w.y - v.y, WHITE);
    DrawRectangleLines(v.x, v.y, w.x - v.x, w.y - v.y, SUBGUI_BG);
  }
}

#endif
#include "instrument_mode.h"

static void display_mode_text() {
  DrawTextSE("INSTRUMENT", screen_width-XMARGIN+5-35, screen_height-YMARGIN-15, 60, BLACK);
  DrawTextSE("INSTRUMENT", screen_width-XMARGIN-35, screen_height-YMARGIN-20, 60, WHITE);
}

void instrument_mode_gui() {
  BeginDrawing();
  ClearBackground(INSTRUMENT_MODE_BG);
  display_mode_text();
  EndDrawing();
}
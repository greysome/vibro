#include "instrument_mode.h"

#define MENU_XMARGIN 50
#define MENU_YMARGIN 50
#define HEADING_COLOR (Color){255,139,135,255}
#define SELECTED_COLOR WHITE
#define UNSELECTED_COLOR LIGHTGRAY

#define CHOOSE_COLOR(cond) (cond) ? SELECTED_COLOR : UNSELECTED_COLOR
#define BIND_LEFT_ON_ROW(n) if (row == n && IsKeyPressed(KEY_LEFT) && !SHIFT_DOWN)
#define BIND_RIGHT_ON_ROW(n) if (row == n && IsKeyPressed(KEY_RIGHT) && !SHIFT_DOWN)

static Instrument instrument = {
  .wave_type = PULSE,
  .pulse_width = 0.5,
  .tri_nes_style = false,
  .saw_nes_style = false
};

static void draw_marker(int y) {
  y += 9; // Make it centered with the text
  DrawTriangle((Vector2){23,y-3}, (Vector2){23,y+7}, (Vector2){33,y+2}, BLACK);
  DrawTriangle((Vector2){20,y-5}, (Vector2){20,y+5}, (Vector2){30,y}, WHITE);
}

static void menu() {
  static int row = 0;
  int x = MENU_XMARGIN;
  int y = MENU_YMARGIN;

  if (row == 0) draw_marker(y);
  x += DrawAndMeasureShadowedText("WAVE", x, y, 20, HEADING_COLOR) + 30;
  x += DrawAndMeasureShadowedText("PULSE", x, y, 20, CHOOSE_COLOR(instrument.wave_type == 0)) + 30;
  x += DrawAndMeasureShadowedText("TRI", x, y, 20, CHOOSE_COLOR(instrument.wave_type == 1)) + 30;
  x += DrawAndMeasureShadowedText("SAW", x, y, 20, CHOOSE_COLOR(instrument.wave_type == 2)) + 30;
#undef option_color
  BIND_LEFT_ON_ROW(0)
    instrument.wave_type = clamp(instrument.wave_type-1, 0, 2);
  BIND_RIGHT_ON_ROW(0)
    instrument.wave_type = clamp(instrument.wave_type+1, 0, 2);

  x = MENU_XMARGIN; y += 30;
  if (row == 1) draw_marker(y);
  switch (instrument.wave_type) {
  case 0:
    x += DrawAndMeasureShadowedText("PULSE WIDTH", x, y, 20, HEADING_COLOR) + 30;
    x += DrawAndMeasureShadowedText(TextFormat("%d%%", (int)(instrument.pulse_width*100)), x, y, 20, WHITE) + 30;
    BIND_LEFT_ON_ROW(1)
      instrument.pulse_width = clamp(instrument.pulse_width-0.05, 0.05, 0.95);
    BIND_RIGHT_ON_ROW(1)
      instrument.pulse_width = clamp(instrument.pulse_width+0.05, 0.05, 0.95);
    break;
  case 1:
    x += DrawAndMeasureShadowedText("NES STYLE?", x, y, 20, HEADING_COLOR) + 30;
    x += DrawAndMeasureShadowedText("YES", x, y, 20, CHOOSE_COLOR(instrument.tri_nes_style)) + 30;
    x += DrawAndMeasureShadowedText("NO", x, y, 20, CHOOSE_COLOR(!instrument.tri_nes_style)) + 30;
    BIND_LEFT_ON_ROW(1)
      instrument.tri_nes_style = true;
    BIND_RIGHT_ON_ROW(1)
      instrument.tri_nes_style = false;
    break;
  case 2:
    x += DrawAndMeasureShadowedText("NES STYLE?", x, y, 20, HEADING_COLOR) + 30;
    x += DrawAndMeasureShadowedText("YES", x, y, 20, CHOOSE_COLOR(instrument.saw_nes_style)) + 30;
    x += DrawAndMeasureShadowedText("NO", x, y, 20, CHOOSE_COLOR(!instrument.saw_nes_style)) + 30;
    BIND_LEFT_ON_ROW(1)
      instrument.saw_nes_style = true;
    BIND_RIGHT_ON_ROW(1)
      instrument.saw_nes_style = false;
    break;
  }

  if (IsKeyPressed(KEY_UP))
    row = clamp(row-1, 0, 1);
  if (IsKeyPressed(KEY_DOWN))
    row = clamp(row+1, 0, 1);
}

void instrument_mode_gui() {
  BeginDrawing();
  ClearBackground((Color){82,64,64,255});
  DrawShadowedTextSE("INSTRUMENT", screen_width-XMARGIN-35, screen_height-YMARGIN-20, 60, WHITE);
  menu();
  EndDrawing();
}

Instrument get_instrument() {
  return instrument;
}
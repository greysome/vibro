#include "instrument_mode.h"

#define MENU_XMARGIN 50
#define MENU_YMARGIN 50
#define HEADING_COLOR (Color){255,139,135,255}
#define SELECTED_COLOR WHITE
#define UNSELECTED_COLOR LIGHTGRAY

#define CHOOSE_COLOR(cond) (cond) ? SELECTED_COLOR : UNSELECTED_COLOR
#define BIND_SCROLLUP_ON_ROW(n) if (row == n && GetMouseWheelMove() < 0)
#define BIND_SCROLLDOWN_ON_ROW(n) if (row == n && GetMouseWheelMove() > 0)
#define BIND_LEFT_ON_ROW(n) if (row == n && IsKeyPressed(KEY_LEFT) && !SHIFT_DOWN)
#define BIND_RIGHT_ON_ROW(n) if (row == n && IsKeyPressed(KEY_RIGHT) && !SHIFT_DOWN)

static Instrument instrument;

static int display_heading(const char *text, int x, int y) {
  return DrawAndMeasureShadowedText(text, x, y, 20, HEADING_COLOR);
}

static int display_option(const char *text, int x, int y, bool should_underline, bool selected) {
  int length = DrawAndMeasureShadowedText(text, x, y, 20, CHOOSE_COLOR(selected));
  if (should_underline && selected) {
    DrawLineEx((Vector2){x+2,y+21}, (Vector2){x+2+length,y+21}, 2, BLACK);
    DrawLineEx((Vector2){x,y+20}, (Vector2){x+length,y+20}, 2, WHITE);
  }
  return length;
}

static int display_text_field(char *str, int x, int y, bool should_update, bool *has_changes) {
  int char_pos = clamp(strlen(str), 0, MAX_STR_LEN);
  int length = display_option(char_pos == 0 ? "NA" : str, x, y, should_update, true);
  if (should_update) {
    int pressed_char = GetCharPressed();
    if (32 <= pressed_char && pressed_char <= 126)
      if (char_pos <= MAX_STR_LEN-1) {
	if (has_changes != NULL)
	  *has_changes = true;
	str[char_pos++] = pressed_char;
      }
    if (IsKeyPressed(KEY_BACKSPACE)) {
      if (char_pos > 0 && char_pos <= MAX_STR_LEN) {
	if (has_changes != NULL)
	  *has_changes = true;
	str[--char_pos] = '\0';
      }
    }
    if (IsKeyPressed(KEY_DELETE)) {
      memset(str, 0, MAX_STR_LEN * sizeof(char));
      char_pos = 0;
    }
  }
  return length;
}

static void menu() {
  static int row = 0;
  int x = MENU_XMARGIN;
  int y = MENU_YMARGIN;

  x += display_heading("NAME", x, y) + 30;
  x += display_text_field(instrument.name, x, y, row == 0, NULL);

  x = MENU_XMARGIN; y += 30;
  x += display_heading("WAVE", x, y) + 30;
  x += display_option("PULSE", x, y, row == 1, instrument.wave_type == PULSE) + 30;
  x += display_option("TRI", x, y, row == 1, instrument.wave_type == TRI) + 30;
  x += display_option("SAW", x, y, row == 1, instrument.wave_type == SAW) + 30;
  x += display_option("SAMPLE", x, y, row == 1, instrument.wave_type == SAMPLE) + 30;
#undef option_color
  BIND_LEFT_ON_ROW(1)
    instrument.wave_type = clamp(instrument.wave_type-1, 0, 3);
  BIND_RIGHT_ON_ROW(1)
    instrument.wave_type = clamp(instrument.wave_type+1, 0, 3);

  x = MENU_XMARGIN; y += 30;
  switch (instrument.wave_type) {
  case PULSE:
    x += display_heading("PULSE WIDTH", x, y) + 30;
    x += display_option(TextFormat("%d%%", (int)(instrument.pulse_width*100)), x, y, row == 2, true) + 30;
    BIND_LEFT_ON_ROW(2)
      instrument.pulse_width = clamp(instrument.pulse_width-0.05, 0.05, 0.95);
    BIND_SCROLLUP_ON_ROW(2)
      instrument.pulse_width = clamp(instrument.pulse_width-0.05, 0.05, 0.95);
    BIND_RIGHT_ON_ROW(2)
      instrument.pulse_width = clamp(instrument.pulse_width+0.05, 0.05, 0.95);
    BIND_SCROLLDOWN_ON_ROW(2)
      instrument.pulse_width = clamp(instrument.pulse_width+0.05, 0.05, 0.95);
    break;

  case TRI:
    x += display_heading("NES STYLE?", x, y) + 30;
    x += display_option("YES", x, y, row == 2, instrument.tri_nes_style) + 30;
    x += display_option("NO", x, y, row == 2, !instrument.tri_nes_style) + 30;
    BIND_LEFT_ON_ROW(2)
      instrument.tri_nes_style = true;
    BIND_RIGHT_ON_ROW(2)
      instrument.tri_nes_style = false;
    break;

  case SAW:
    x += display_heading("NES STYLE?", x, y) + 30;
    x += display_option("YES", x, y, row == 2, instrument.saw_nes_style) + 30;
    x += display_option("NO", x, y, row == 2, !instrument.saw_nes_style) + 30;
    BIND_LEFT_ON_ROW(2)
      instrument.saw_nes_style = true;
    BIND_RIGHT_ON_ROW(2)
      instrument.saw_nes_style = false;
    break;

  case SAMPLE:
    x += display_heading("SAMPLE FILE", x, y) + 30;
    x += display_text_field(instrument.sample_path, x, y, row == 2, &instrument.sample_changed) + 30;

    x = MENU_XMARGIN; y += 30;
    x += display_heading("PITCH OFFSET", x, y) + 30;
    x += display_option(TextFormat("%.2f", instrument.sample_pitch_modifier), x, y, row == 3, true) + 30;
    BIND_LEFT_ON_ROW(3)
      instrument.sample_pitch_modifier = clamp(instrument.sample_pitch_modifier - 0.05, 0.5, 2.0);
    BIND_SCROLLUP_ON_ROW(3)
      instrument.sample_pitch_modifier = clamp(instrument.sample_pitch_modifier - 0.01, 0.5, 2.0);
    BIND_RIGHT_ON_ROW(3)
      instrument.sample_pitch_modifier = clamp(instrument.sample_pitch_modifier + 0.05, 0.5, 2.0);
    BIND_SCROLLDOWN_ON_ROW(3)
      instrument.sample_pitch_modifier = clamp(instrument.sample_pitch_modifier + 0.01, 0.5, 2.0);

    x = MENU_XMARGIN; y += 30;
    x += display_heading("PLAY CONTINUOUSLY?", x, y) + 30;
    x += display_option("YES", x, y, row == 4, instrument.sample_play_continuously) + 30;
    x += display_option("NO", x, y, row == 4, !instrument.sample_play_continuously) + 30;
    BIND_LEFT_ON_ROW(4)
      instrument.sample_play_continuously = true;
    BIND_RIGHT_ON_ROW(4)
      instrument.sample_play_continuously = false;

    x = MENU_XMARGIN; y += 30;
    x += display_heading("STOP ON RELEASE?", x, y) + 30;
    x += display_option("YES", x, y, row == 5, instrument.sample_stop_on_release) + 30;
    x += display_option("NO", x, y, row == 5, !instrument.sample_stop_on_release) + 30;
    BIND_LEFT_ON_ROW(5)
      instrument.sample_stop_on_release = true;
    BIND_RIGHT_ON_ROW(5)
      instrument.sample_stop_on_release = false;
    break;
  }

  int num_rows;
  switch (instrument.wave_type) {
  case PULSE: case TRI: case SAW:
    num_rows = 3;
    break;
  case SAMPLE:
    num_rows = 6;
    break;
  }
  if (IsKeyPressed(KEY_UP))
    row = clamp(row-1, 0, num_rows-1);
  if (IsKeyPressed(KEY_DOWN))
    row = clamp(row+1, 0, num_rows-1);
}

void init_instrument() {
  strncpy(instrument.name, "Instrument 1", 12);
  instrument.wave_type = PULSE;
  instrument.pulse_width = 0.5;
  instrument.tri_nes_style = false;
  instrument.saw_nes_style = false;
  strncpy(instrument.sample_path, "", 0);
  instrument.sample_changed = false;
  instrument.sample_ready = false;
  instrument.sample_pitch_modifier = 1;
  instrument.sample_play_continuously = false;
  instrument.sample_stop_on_release = true;
  memset(&instrument.sample, 0, sizeof(Sound));
  instrument.sample_rate = 0;
  instrument.sample_data_length = 0;
  instrument.sample_data = NULL;
}

void reload_instrument_sample_if_changed() {
  if (instrument.sample_changed) {
    UnloadWaveSamples(instrument.sample_data);
    UnloadSound(instrument.sample);
    const char *filepath = TextFormat("%ssamples/%s", GetApplicationDirectory(), instrument.sample_path);
    instrument.sample = LoadSound(filepath);
    instrument.sample_ready = IsSoundReady(instrument.sample);
    if (instrument.sample_ready) {
      Wave wave = LoadWave(filepath);
      instrument.sample_rate = wave.sampleRate;
      instrument.sample_data_length = wave.frameCount;
      instrument.sample_data = LoadWaveSamples(wave);
      UnloadWave(wave);
    }
  }
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
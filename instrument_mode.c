#include "instrument_mode.h"
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

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

typedef struct {
  int note;
  Sample sample;
} MultisampleEntry;

typedef struct {
  Sample sample;
  MultisampleEntry *multisample_entries;
  int selected_column; // 0, 1, 2 or 3
} InstrumentModeState;

static Instrument instrument;
static InstrumentModeState mode_state;

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

static int display_text_field(char *str, int x, int y, bool should_update) {
  int char_pos = clamp(strlen(str), 0, MAX_STR_LEN);
  int length = display_option(char_pos == 0 ? "NA" : str, x, y, should_update, true);
  if (should_update) {
    int pressed_char = GetCharPressed();
    if (32 <= pressed_char && pressed_char <= 126)
      if (char_pos <= MAX_STR_LEN-1)
	str[char_pos++] = pressed_char;
    if (IsKeyPressed(KEY_BACKSPACE)) {
      if (char_pos > 0 && char_pos <= MAX_STR_LEN)
	str[--char_pos] = '\0';
    }
    if (IsKeyPressed(KEY_DELETE)) {
      memset(str, 0, MAX_STR_LEN * sizeof(char));
      char_pos = 0;
    }
  }
  return length;
}

static void populate_sample_data(Sample *sample) {
  const char *filepath = TextFormat("%ssamples/%s", GetApplicationDirectory(), sample->path);
  sample->sound = LoadSound(filepath);
  sample->is_ready = IsSoundReady(sample->sound);
  if (sample->is_ready) {
    sample->is_alias = false;
    Wave wave = LoadWave(filepath);
    sample->sample_rate = wave.sampleRate;
    sample->num_frames = wave.frameCount;
    sample->data = LoadWaveSamples(wave);
    UnloadWave(wave);
  }
}

int char_to_note(char c) {
  switch (c) {
  case 'a': return 0;
  case 'z': return 1;
  case 's': return 2;
  case 'x': return 3;
  case 'd': return 4;
  case 'c': return 5;
  case 'v': return 6;
  case 'g': return 7;
  case 'b': return 8;
  case 'h': return 9;
  case 'n': return 10;
  case 'j': return 11;
  case 'm': case '1': return 12;
  case ',': case 'q': return 13;
  case 'l': case '2': return 14;
  case '.': case 'w': return 15;
  case ';': case '3': return 16;
  case '/': case 'e': return 17;
  case '\'': case 'r': return 18;
  case '5': return 19;
  case 't': return 20;
  case '6': return 21;
  case 'y': return 22;
  case '7': return 23;
  case 'u': return 24;
  case 'i': return 25;
  case '9': return 26;
  case 'o': return 27;
  case '0': return 28;
  case 'p': return 29;
  case '[': return 30;
  case '=': return 31;
  case ']': return 32;
  default: return NIL;
  }
}

char *note_to_chars(int note) {
  switch (note) {
  case 0: return "A";
  case 1: return "Z";
  case 2: return "S";
  case 3: return "X";
  case 4: return "D";
  case 5: return "C";
  case 6: return "V";
  case 7: return "G";
  case 8: return "B";
  case 9: return "H";
  case 10: return "N";
  case 11: return "J";
  case 12: return "M or 1";
  case 13: return ", or Q";
  case 14: return "L or 2";
  case 15: return ". or W";
  case 16: return "; or /";
  case 17: return "/ or A";
  case 18: return "' or R";
  case 19: return "5";
  case 20: return "T";
  case 21: return "6";
  case 22: return "Y";
  case 23: return "7";
  case 24: return "U";
  case 25: return "I";
  case 26: return "9";
  case 27: return "O";
  case 28: return "0";
  case 29: return "P";
  case 30: return "[";
  case 31: return "=";
  case 32: return "]";
  default: return NULL;
  }
}

void init_sample_fields(Sample *sample) {
  memset(sample->path, 0, MAX_STR_LEN * sizeof(char));
  sample->pitch_modifier = 1;
  sample->volume_modifier = 1;
  sample->play_continuously = false;
  sample->stop_on_release = false;

  sample->is_ready = false;
  sample->sample_rate = NIL;
  sample->num_frames = NIL;

  sample->is_alias = false;
  memset(&sample->sound, 0, sizeof(Sound));
  sample->data = NULL;
}

void init_instrument() {
  strncpy(instrument.name, "Instrument 1", 12);
  instrument.wave_type = MULTISAMPLE;
  instrument.pulse_width = 0.5;
  instrument.tri_nes_style = false;
  instrument.saw_nes_style = false;
  for (int i = 0; i < NOTETABLE_SIZE; i++)
    init_sample_fields(&instrument.samples[i]);
}

void load_instrument_mode_state() {
  mode_state.sample = instrument.samples[0];

  if (arrlenu(mode_state.multisample_entries) > 0)
    arrfree(mode_state.multisample_entries);
  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    Sample sample = instrument.samples[note];
    if (strnlen(sample.path, MAX_STR_LEN) == 0)
      continue;
    MultisampleEntry entry = {.note = note, .sample = sample};
    arrput(mode_state.multisample_entries, entry);
  }
}

void cleanup_instrument() {
  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    Sample sample = instrument.samples[note];
    if (!sample.is_ready)
      continue;
    if (instrument.samples[note].is_alias)
      UnloadSoundAlias(instrument.samples[note].sound);
    else {
      UnloadSound(instrument.samples[note].sound);
      UnloadWaveSamples((float *)instrument.samples[note].data);
    }
    init_sample_fields(&instrument.samples[note]);
  }
}

void cleanup_instrument_mode_state() {
  if (mode_state.multisample_entries != NULL)
    arrfree(mode_state.multisample_entries);
}

void commit_instrument_mode_changes() {
  cleanup_instrument();
  if (instrument.wave_type == SAMPLE) {
    instrument.samples[0] = mode_state.sample;
    populate_sample_data(&instrument.samples[0]);
    // Make sample aliases
    if (instrument.samples[0].is_ready) {
      for (int note = 1; note < NOTETABLE_SIZE; note++) {
	// Copy all fields over except make the sound an alias
	instrument.samples[note] = instrument.samples[0];
	instrument.samples[note].is_alias = true;
	instrument.samples[note].sound = LoadSoundAlias(instrument.samples[0].sound);
      }
    }
  }
  else if (instrument.wave_type == MULTISAMPLE) {
    for (int i = 0; i < arrlenu(mode_state.multisample_entries); i++) {
      MultisampleEntry entry = mode_state.multisample_entries[i];
      if (entry.note != NIL) {
	instrument.samples[entry.note] = entry.sample;
	populate_sample_data(&instrument.samples[entry.note]);
      }
    }
    arrfree(mode_state.multisample_entries);
  }
}

static void add_multisample_entry() {
  Sample sample;
  init_sample_fields(&sample);
  MultisampleEntry entry = {.note = NIL, .sample = sample};
  arrput(mode_state.multisample_entries, entry);
}

static void multisample_submenu(int *row, int *x, int *y) {
  if (IsKeyPressed(KEY_LEFT))
    mode_state.selected_column = clamp(mode_state.selected_column-1, 0, 3);
  if (IsKeyPressed(KEY_RIGHT))
    mode_state.selected_column = clamp(mode_state.selected_column+1, 0, 3);

  int len = arrlenu(mode_state.multisample_entries);
  int selected_entry = *row - 2;

  if (selected_entry >= 0 && selected_entry < arrlenu(mode_state.multisample_entries)) {
    if (mode_state.selected_column == 0) {  // KEYBIND
      int pressed_char = GetCharPressed();
      int note = char_to_note(pressed_char);
      if (note != NIL) {
	bool already_bound = false;
	for (int i = 0; i < len; i++) {
	  if (note == mode_state.multisample_entries[i].note) {
	    already_bound = true;
	    break;
	  }
	}
	if (!already_bound)
	  mode_state.multisample_entries[selected_entry].note = note;
      }
    }
    else if (mode_state.selected_column == 1) {  // SAMPLE PATH
    }
    else if (mode_state.selected_column == 2) {  // DELETE
      if (IsKeyPressed(KEY_ENTER)) {
	arrdel(mode_state.multisample_entries, selected_entry);
	(*row)--;
      }
    }
    else if (mode_state.selected_column == 3) {  // ADD
      if (IsKeyPressed(KEY_ENTER)) {
	add_multisample_entry();
	*row = arrlenu(mode_state.multisample_entries) + 1;
      }
    }
  }

  if (arrlenu(mode_state.multisample_entries) == 0) {
    add_multisample_entry();
    *row = arrlenu(mode_state.multisample_entries) + 1;
  }
  for (int i = 0; i < arrlenu(mode_state.multisample_entries); i++) {
    MultisampleEntry entry = mode_state.multisample_entries[i];
    *x += display_heading("BIND", *x, *y) + 30;
    char *chars = note_to_chars(entry.note);
    if (chars == NULL)
      chars = "NA\0";
    *x += display_option(chars, *x, *y, selected_entry == i && mode_state.selected_column == 0, true) + 30;
    *x += display_heading("TO", *x, *y) + 30;
    *x += display_text_field(mode_state.multisample_entries[i].sample.path, *x, *y, selected_entry == i && mode_state.selected_column == 1) + 60;
    *x += display_option("DELETE", *x, *y, selected_entry == i && mode_state.selected_column == 2, true) + 30;
    *x += display_option("ADD", *x, *y, selected_entry == i && mode_state.selected_column == 3, true) + 30;
    *x = MENU_XMARGIN; *y += 30;
  }
  y -= 30;
}

static void menu() {
  static int row = 0;
  int x = MENU_XMARGIN;
  int y = MENU_YMARGIN;

  x += display_heading("NAME", x, y) + 30;
  x += display_text_field(instrument.name, x, y, row == 0);

  x = MENU_XMARGIN; y += 30;
  x += display_heading("WAVE", x, y) + 30;
  x += display_option("PULSE", x, y, row == 1, instrument.wave_type == PULSE) + 30;
  x += display_option("TRI", x, y, row == 1, instrument.wave_type == TRI) + 30;
  x += display_option("SAW", x, y, row == 1, instrument.wave_type == SAW) + 30;
  x += display_option("SAMPLE", x, y, row == 1, instrument.wave_type == SAMPLE) + 30;
  x += display_option("MULTISAMPLE", x, y, row == 1, instrument.wave_type == MULTISAMPLE) + 30;
  BIND_LEFT_ON_ROW(1)
    instrument.wave_type = clamp(instrument.wave_type-1, 0, 4);
  BIND_RIGHT_ON_ROW(1)
    instrument.wave_type = clamp(instrument.wave_type+1, 0, 4);

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
    x += display_text_field(mode_state.sample.path, x, y, row == 2) + 30;

    x = MENU_XMARGIN; y += 30;
    x += display_heading("PITCH MODIFIER", x, y) + 30;
    x += display_option(TextFormat("%.2f", mode_state.sample.pitch_modifier), x, y, row == 3, true) + 30;
    BIND_LEFT_ON_ROW(3)
      mode_state.sample.pitch_modifier = clamp(mode_state.sample.pitch_modifier - 0.05, 0.5, 2.0);
    BIND_SCROLLUP_ON_ROW(3)
      mode_state.sample.pitch_modifier = clamp(mode_state.sample.pitch_modifier - 0.01, 0.5, 2.0);
    BIND_RIGHT_ON_ROW(3)
      mode_state.sample.pitch_modifier = clamp(mode_state.sample.pitch_modifier + 0.05, 0.5, 2.0);
    BIND_SCROLLDOWN_ON_ROW(3)
      mode_state.sample.pitch_modifier = clamp(mode_state.sample.pitch_modifier + 0.01, 0.5, 2.0);

    x = MENU_XMARGIN; y += 30;
    x += display_heading("VOLUME MODIFIER", x, y) + 30;
    x += display_option(TextFormat("%.2f", mode_state.sample.volume_modifier), x, y, row == 4, true) + 30;
    BIND_LEFT_ON_ROW(4)
      mode_state.sample.volume_modifier = clamp(mode_state.sample.volume_modifier - 0.05, 0.5, 2.0);
    BIND_SCROLLUP_ON_ROW(4)
      mode_state.sample.volume_modifier = clamp(mode_state.sample.volume_modifier - 0.01, 0.5, 2.0);
    BIND_RIGHT_ON_ROW(4)
      mode_state.sample.volume_modifier = clamp(mode_state.sample.volume_modifier + 0.05, 0.5, 2.0);
    BIND_SCROLLDOWN_ON_ROW(4)
      mode_state.sample.volume_modifier = clamp(mode_state.sample.volume_modifier + 0.01, 0.5, 2.0);

    x = MENU_XMARGIN; y += 30;
    x += display_heading("PLAY CONTINUOUSLY?", x, y) + 30;
    x += display_option("YES", x, y, row == 5, mode_state.sample.play_continuously) + 30;
    x += display_option("NO", x, y, row == 5, !mode_state.sample.play_continuously) + 30;
    BIND_LEFT_ON_ROW(5)
      mode_state.sample.play_continuously = true;
    BIND_RIGHT_ON_ROW(5)
      mode_state.sample.play_continuously = false;

    x = MENU_XMARGIN; y += 30;
    x += display_heading("STOP ON RELEASE?", x, y) + 30;
    x += display_option("YES", x, y, row == 6, mode_state.sample.stop_on_release) + 30;
    x += display_option("NO", x, y, row == 6, !mode_state.sample.stop_on_release) + 30;
    BIND_LEFT_ON_ROW(6)
      mode_state.sample.stop_on_release = true;
    BIND_RIGHT_ON_ROW(6)
      mode_state.sample.stop_on_release = false;
    break;

  case MULTISAMPLE:
    multisample_submenu(&row, &x, &y);
    break;
  }

  int num_rows;
  switch (instrument.wave_type) {
  case PULSE: case TRI: case SAW:
    num_rows = 3;
    break;
  case SAMPLE:
    num_rows = 7;
    break;
  case MULTISAMPLE:
    num_rows = arrlenu(mode_state.multisample_entries) + 2;
    break;
  }

  if (IsKeyPressed(KEY_UP))
    row = clamp(row-1, 0, num_rows-1);
  if (IsKeyPressed(KEY_DOWN))
    row = clamp(row+1, 0, num_rows-1);
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
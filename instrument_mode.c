#include "instrument_mode.h"

#define MENU_XMARGIN 50
#define MENU_YMARGIN 50
#define HEADING_COLOR (Color){255,139,135,255}
#define SELECTED_COLOR WHITE
#define UNSELECTED_COLOR LIGHTGRAY

#define CHOOSE_COLOR(cond) (cond) ? SELECTED_COLOR : UNSELECTED_COLOR

typedef struct {
  int note;
  // The user can expand the entry to show all sample parameters, e.g. pitch modifier
  bool is_expanded;
  Sample sample;
} MultisampleEntry;

typedef struct {
  Sample sample;
  MultisampleEntry *multisample_entries;
} InstrumentModeState;

static InstrumentModeState mode_state;
static int cur_row = 0;  // Current row of the GUI

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

static int display_text_field(char *str, int x, int y, bool should_update, bool selected) {
  int char_pos = clamp(strlen(str), 0, MAX_STR_LEN);
  int length = display_option(char_pos == 0 ? "NA" : str, x, y, should_update, selected);
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

static int char_to_note(char c) {
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

static char *note_to_chars(int note) {
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

static void init_adsr_params(ADSRParams *adsr) {
  adsr->attack_frames = 10;
  adsr->decay_frames = 5;
  adsr->sustain_vol = 0.7;
  adsr->release_frames = 20;
}

static void init_sample_fields(Sample *sample) {
  memset(sample->path, 0, MAX_STR_LEN * sizeof(char));
  sample->pitch_modifier = 1;
  sample->volume_modifier = 1;
  sample->play_continuously = false;
  sample->stop_on_release = false;
  init_adsr_params(&sample->adsr);

  sample->is_ready = false;
  sample->sample_rate = NIL;
  sample->num_frames = NIL;

  sample->is_alias = false;
  memset(&sample->sound, 0, sizeof(Sound));
  sample->data = NULL;
}

void load_instrument_mode_state(int instrument_num) {
  Instrument instrument = get_instruments()[instrument_num];
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

void cleanup_instrument_mode_state() {
  if (mode_state.multisample_entries != NULL)
    arrfree(mode_state.multisample_entries);
}

static void load_instrument(Instrument *instrument) {
  if (instrument->type == SAMPLE) {
    instrument->samples[0] = mode_state.sample;
    populate_sample_data(&instrument->samples[0]);
    // Make sample aliases
    if (instrument->samples[0].is_ready) {
      for (int note = 1; note < NOTETABLE_SIZE; note++) {
	// Copy all fields over except make the sound an alias
	instrument->samples[note] = instrument->samples[0];
	instrument->samples[note].is_alias = true;
	instrument->samples[note].sound = LoadSoundAlias(instrument->samples[0].sound);
      }
    }
  }
  else if (instrument->type == MULTISAMPLE) {
    for (int i = 0; i < (int)arrlenu(mode_state.multisample_entries); i++) {
      MultisampleEntry entry = mode_state.multisample_entries[i];
      if (entry.note != NIL) {
	instrument->samples[entry.note] = entry.sample;
	populate_sample_data(&instrument->samples[entry.note]);
      }
    }
    arrfree(mode_state.multisample_entries);
  }
}

void commit_instrument_mode_changes() {
  cleanup_instruments();
  for (int i = 0; i < get_num_instruments(); i++)
    load_instrument(&get_instruments()[i]);
}

static void add_multisample_entry() {
  Sample sample;
  init_sample_fields(&sample);
  MultisampleEntry entry = {.note = NIL, .sample = sample, .is_expanded = false};
  arrput(mode_state.multisample_entries, entry);
}


#define ON_ENTRY_AND_ROW(i,row) (cur_entry == i && cur_entryrow == row)
#define BIND_SCROLLUP_ON_ENTRY_ROW(i,row) if (ON_ENTRY_AND_ROW(i,row) && GetMouseWheelMove() < 0)
#define BIND_SCROLLDOWN_ON_ENTRY_ROW(i,row) if (ON_ENTRY_AND_ROW(i,row) && GetMouseWheelMove() > 0)
#define BIND_LEFT_ON_ENTRY_ROW(i,row) if (ON_ENTRY_AND_ROW(i,row) && IsKeyPressed(KEY_LEFT) && !SHIFT_DOWN)
#define BIND_RIGHT_ON_ENTRY_ROW(i,row) if (ON_ENTRY_AND_ROW(i,row) && IsKeyPressed(KEY_RIGHT) && !SHIFT_DOWN)

// NIL value means that the current row is either instrument name or type
static int cur_entry = NIL;
// When expanded, an entry takes up multiple rows showing the sample parameters.
static int cur_entryrow = NIL;
// Only applicable when cur_entryrow = 0.
// The first row of each multisample entry looks like this:
// BIND [KEY] TO [SAMPLE PATH]      [ADD] [DELETE] [+/-]
//        0            1              2      3       4
//        ^ cur_col
static int cur_col = 0;

void reset_entryrow() {
  cur_entryrow = cur_entryrow == NIL ? NIL : 0;
}

static void multisample_update_bool_field(int entry, int entry_row, bool *field) {
  BIND_LEFT_ON_ENTRY_ROW(entry, entry_row)
    *field = true;
  BIND_RIGHT_ON_ENTRY_ROW(entry, entry_row)
    *field = false;
}

static void multisample_update_int_field(int entry, int entry_row, int *field, int min, int max, int small_offset, int large_offset) {
  BIND_LEFT_ON_ENTRY_ROW(entry, entry_row)
    *field = clamp(*field - large_offset, min, max);
  BIND_SCROLLUP_ON_ENTRY_ROW(entry, entry_row)
    *field = clamp(*field - small_offset, min, max);
  BIND_RIGHT_ON_ENTRY_ROW(entry, entry_row)
    *field = clamp(*field + large_offset, min, max);
  BIND_SCROLLDOWN_ON_ENTRY_ROW(entry, entry_row)
    *field = clamp(*field + small_offset, min, max);
}

static void multisample_update_float_field(int entry, int entry_row, float *field, float min, float max, float small_offset, float large_offset) {
  BIND_LEFT_ON_ENTRY_ROW(entry, entry_row)
    *field = fclamp(*field - large_offset, min, max);
  BIND_SCROLLUP_ON_ENTRY_ROW(entry, entry_row)
    *field = fclamp(*field - small_offset, min, max);
  BIND_RIGHT_ON_ENTRY_ROW(entry, entry_row)
    *field = fclamp(*field + large_offset, min, max);
  BIND_SCROLLDOWN_ON_ENTRY_ROW(entry, entry_row)
    *field = fclamp(*field + small_offset, min, max);
}

static void multisample_submenu(int *x, int *y) {
  if (cur_entryrow == 0) {
    if (IsKeyPressed(KEY_LEFT))
      cur_col = clamp(cur_col-1, 0, 4);
    if (IsKeyPressed(KEY_RIGHT))
      cur_col = clamp(cur_col+1, 0, 4);
  }

  int num_entries = arrlenu(mode_state.multisample_entries);

  if (cur_entry >= 0 && cur_entry < num_entries) {
    if (cur_entryrow == 0) {
      if (cur_col == 0) {  // KEYBIND
	int pressed_char = GetCharPressed();
	int note = char_to_note(pressed_char);
	if (note != NIL) {
	  bool already_bound = false;
	  for (int i = 0; i < num_entries; i++) {
	    if (note == mode_state.multisample_entries[i].note) {
	      already_bound = true;
	      break;
	    }
	  }
	  if (!already_bound)
	    mode_state.multisample_entries[cur_entry].note = note;
	}
      }
      else if (cur_col == 1) {  // SAMPLE PATH
      }
      else if (cur_col == 2) {  // +/-
	if (IsKeyPressed(KEY_ENTER))
	  mode_state.multisample_entries[cur_entry].is_expanded = !mode_state.multisample_entries[cur_entry].is_expanded;
      }
      else if (cur_col == 3) {  // ADD
	if (IsKeyPressed(KEY_ENTER)) {
	  add_multisample_entry();
	  //cur_row = arrlenu(mode_state.multisample_entries) + 1;
	}
      }
      else if (cur_col == 4) {  // DELETE
	if (IsKeyPressed(KEY_ENTER)) {
	  arrdel(mode_state.multisample_entries, cur_entry);
	  if (cur_entry > 0) {
	    cur_row -= cur_entryrow+1;
	    cur_entry--;
	  }
	  cur_entryrow = 0;
	}
      }
    }
  }

  if (num_entries == 0)
    add_multisample_entry();

  // Besides storing the total number of rows, this also keeps track of the current row
  // when building the GUI.
  int num_rows = 2;
  for (int i = 0; i < num_entries; i++) {
    MultisampleEntry entry = mode_state.multisample_entries[i];

    *x = MENU_XMARGIN;
    *x += display_heading("BIND", *x, *y) + 30;
    char *chars = note_to_chars(entry.note);
    if (chars == NULL)
      chars = "NA";
    *x += display_option(chars, *x, *y, ON_ENTRY_AND_ROW(i, 0) && cur_col == 0, true) + 30;
    *x += display_heading("TO", *x, *y) + 30;
    *x += display_text_field(mode_state.multisample_entries[i].sample.path, *x, *y, ON_ENTRY_AND_ROW(i, 0) && cur_col == 1, true) + 60;
    *x += display_option(entry.is_expanded ? "-" : "+", *x, *y, ON_ENTRY_AND_ROW(i, 0) && cur_col == 2, true) + 30;
    *x += display_option("ADD", *x, *y, ON_ENTRY_AND_ROW(i, 0) && cur_col == 3, true) + 30;
    *x += display_option("DELETE", *x, *y, ON_ENTRY_AND_ROW(i, 0) && cur_col == 4, true) + 30;
    if (!entry.is_expanded) {
      *x = MENU_XMARGIN; *y += 30;
    }
    num_rows++;

    if (entry.is_expanded) {
      *x = 2*MENU_XMARGIN; *y += 30;
      *x += display_heading("PITCH MODIFIER", *x, *y) + 30;
      *x += display_option(TextFormat("%.2f", entry.sample.pitch_modifier), *x, *y, ON_ENTRY_AND_ROW(i, 1), true) + 30;
      multisample_update_float_field(i, 1, &mode_state.multisample_entries[i].sample.pitch_modifier, 0.5, 2.0, 0.01, 0.05);

      *x = 2*MENU_XMARGIN; *y += 30;
      *x += display_heading("VOLUME MODIFIER", *x, *y) + 30;
      *x += display_option(TextFormat("%.2f", entry.sample.volume_modifier), *x, *y, ON_ENTRY_AND_ROW(i, 2), true) + 30;
      multisample_update_float_field(i, 2, &mode_state.multisample_entries[i].sample.volume_modifier, 0.5, 2.0, 0.01, 0.05);

      *x = 2*MENU_XMARGIN; *y += 30;
      *x += display_heading("PLAY CONTINUOUSLY?", *x, *y) + 30;
      *x += display_option("YES", *x, *y, ON_ENTRY_AND_ROW(i, 3), entry.sample.play_continuously) + 30;
      *x += display_option("NO", *x, *y, ON_ENTRY_AND_ROW(i, 3), !entry.sample.play_continuously) + 30;
      multisample_update_bool_field(i, 3, &mode_state.multisample_entries[i].sample.play_continuously);

      *x = 2*MENU_XMARGIN; *y += 30;
      *x += display_heading("STOP ON RELEASE?", *x, *y) + 30;
      *x += display_option("YES", *x, *y, ON_ENTRY_AND_ROW(i, 4), entry.sample.stop_on_release) + 30;
      *x += display_option("NO", *x, *y, ON_ENTRY_AND_ROW(i, 4), !entry.sample.stop_on_release) + 30;
      multisample_update_bool_field(i, 4, &mode_state.multisample_entries[i].sample.stop_on_release);

      *x = 2*MENU_XMARGIN; *y += 30;
      *x += display_heading("ATTACK FRAMES", *x, *y) + 30;
      *x += display_option(TextFormat("%d", entry.sample.adsr.attack_frames), *x, *y, ON_ENTRY_AND_ROW(i, 5), true) + 30;
      multisample_update_int_field(i, 5, &mode_state.multisample_entries[i].sample.adsr.attack_frames, 0, 100, 1, 5);

      *x = 2*MENU_XMARGIN; *y += 30;
      *x += display_heading("DECAY FRAMES", *x, *y) + 30;
      *x += display_option(TextFormat("%d", entry.sample.adsr.decay_frames), *x, *y, ON_ENTRY_AND_ROW(i, 6), true) + 30;
      multisample_update_int_field(i, 6, &mode_state.multisample_entries[i].sample.adsr.decay_frames, 0, 100, 1, 5);

      *x = 2*MENU_XMARGIN; *y += 30;
      *x += display_heading("SUSTAIN VOL", *x, *y) + 30;
      *x += display_option(TextFormat("%d%%", (int)(entry.sample.adsr.sustain_vol*100)), *x, *y, ON_ENTRY_AND_ROW(i, 7), true) + 30;
      multisample_update_float_field(i, 7, &mode_state.multisample_entries[i].sample.adsr.sustain_vol, 0, 1, 0.01, 0.05);

      *x = 2*MENU_XMARGIN; *y += 30;
      *x += display_heading("RELEASE FRAMES", *x, *y) + 30;
      *x += display_option(TextFormat("%d", entry.sample.adsr.release_frames), *x, *y, ON_ENTRY_AND_ROW(i, 8), true) + 30;
      multisample_update_int_field(i, 8, &mode_state.multisample_entries[i].sample.adsr.release_frames, 0, 100, 1, 5);

      *y += 30;
      num_rows += 8;
    }
  }
  *y -= 30;

  bool is_expanded = false;
  if (cur_entry != NIL)
    is_expanded = mode_state.multisample_entries[cur_entry].is_expanded;

  if (IsKeyPressed(KEY_UP) && !SHIFT_DOWN) {
    if (cur_entry == 0 && cur_entryrow == 0) {
      cur_entry = NIL; cur_entryrow = NIL;
    }
    else if (cur_entry > 0 && cur_entryrow == 0) {
      cur_entry--;
      cur_entryrow = mode_state.multisample_entries[cur_entry].is_expanded ? 8 : 0;
    }
    else if (cur_entryrow > 0) {
      cur_entryrow--;
    }
    cur_row = clamp(cur_row-1, -1, num_rows-1);
  }
  if (IsKeyPressed(KEY_DOWN) && !SHIFT_DOWN) {
    if (cur_row < 1) {
      cur_entry = NIL; cur_entryrow = NIL;
    }
    else if (cur_row == 1) {
      cur_entry = 0; cur_entryrow = 0;
    }
    else if (cur_entryrow == 0 && !is_expanded) {
      if (cur_entry < num_entries - 1)
	cur_entry++;
    }
    else if (cur_entryrow < 8 && is_expanded) {
      cur_entryrow++;
    }
    else if (cur_entryrow == 8 && is_expanded) {
      if (cur_entry < num_entries - 1) {
	cur_entry++; cur_entryrow = 0;
      }
    }
    cur_row = clamp(cur_row+1, -1, num_rows-1);
  }
}

#define BIND_SCROLLUP_ON_ROW(n) if (cur_row == n && GetMouseWheelMove() < 0)
#define BIND_SCROLLDOWN_ON_ROW(n) if (cur_row == n && GetMouseWheelMove() > 0)
#define BIND_LEFT_ON_ROW(n) if (cur_row == n && IsKeyPressed(KEY_LEFT) && !SHIFT_DOWN)
#define BIND_RIGHT_ON_ROW(n) if (cur_row == n && IsKeyPressed(KEY_RIGHT) && !SHIFT_DOWN)

static void update_bool_field(int row, bool *field) {
  BIND_LEFT_ON_ROW(row)
    *field = true;
  BIND_RIGHT_ON_ROW(row)
    *field = false;
}

static void update_int_field(int row, int *field, int min, int max, int small_offset, int large_offset) {
  BIND_LEFT_ON_ROW(row)
    *field = clamp(*field - large_offset, min, max);
  BIND_SCROLLUP_ON_ROW(row)
    *field = clamp(*field - small_offset, min, max);
  BIND_RIGHT_ON_ROW(row)
    *field = clamp(*field + large_offset, min, max);
  BIND_SCROLLDOWN_ON_ROW(row)
    *field = clamp(*field + small_offset, min, max);
}

static void update_float_field(int row, float *field, float min, float max, float small_offset, float large_offset) {
  BIND_LEFT_ON_ROW(row)
    *field = fclamp(*field - large_offset, min, max);
  BIND_SCROLLUP_ON_ROW(row)
    *field = fclamp(*field - small_offset, min, max);
  BIND_RIGHT_ON_ROW(row)
    *field = fclamp(*field + large_offset, min, max);
  BIND_SCROLLDOWN_ON_ROW(row)
    *field = fclamp(*field + small_offset, min, max);
}

static void adsr_submenu(int start_row, int *x, int *y) {
  Instrument *instrument = get_cur_instrument();
  *x = MENU_XMARGIN; *y += 50;
  *x += display_heading("ATTACK FRAMES", *x, *y) + 30;
  *x += display_option(TextFormat("%d", instrument->adsr.attack_frames), *x, *y, cur_row == start_row, true) + 30;
  *x = MENU_XMARGIN; *y += 30;
  update_int_field(start_row, &instrument->adsr.attack_frames, 0, 100, 1, 5);

  *x += display_heading("DECAY FRAMES", *x, *y) + 30;
  *x += display_option(TextFormat("%d", instrument->adsr.decay_frames), *x, *y, cur_row == start_row+1, true) + 30;
  *x = MENU_XMARGIN; *y += 30;
  update_int_field(start_row+1, &instrument->adsr.decay_frames, 0, 100, 1, 5);

  *x += display_heading("SUSTAIN VOL", *x, *y) + 30;
  *x += display_option(TextFormat("%d%%", (int)(instrument->adsr.sustain_vol*100)), *x, *y, cur_row == start_row+2, true) + 30;
  *x = MENU_XMARGIN; *y += 30;
  update_float_field(start_row+2, &instrument->adsr.sustain_vol, 0, 1, 0.01, 0.05);

  *x += display_heading("RELEASE FRAMES", *x, *y) + 30;
  *x += display_option(TextFormat("%d", instrument->adsr.release_frames), *x, *y, cur_row == start_row+3, true) + 30;
  *x = MENU_XMARGIN; *y += 30;
  update_int_field(start_row+3, &instrument->adsr.release_frames, 0, 100, 1, 5);
}

static void menu() {
  //printf("row %d, entry %d:%d, cur_col %d\n", cur_row, cur_entry, cur_entryrow, cur_col);
  int x = MENU_XMARGIN;
  int y = MENU_YMARGIN;
  Instrument *instrument = get_cur_instrument();

  // I start from row -1 because this was the last row that I added and I am too lazy to change the numbers of the other rows
  for (int i = 0; i < get_num_instruments(); i++)
    x += display_text_field(get_instruments()[i].name, x, y, cur_row == -1 && get_cur_instrument_idx() == i, get_cur_instrument_idx() == i) + 30;
  BIND_LEFT_ON_ROW(-1)
    select_previous_instrument();
  BIND_RIGHT_ON_ROW(-1)
    select_next_instrument();
  x = MENU_XMARGIN; y += 30;

  static int first_row_option = 0;
  x += display_option("ADD", x, y, cur_row == 0 && first_row_option == 0, true) + 30;
  x += display_option("DELETE", x, y, cur_row == 0 && first_row_option == 1, true) + 30;
  BIND_LEFT_ON_ROW(0)
    first_row_option = 0;
  BIND_RIGHT_ON_ROW(0)
    first_row_option = 1;
  if (cur_row == 0 && IsKeyPressed(KEY_ENTER)) {
    if (first_row_option == 0) {
      add_instrument();
      increment_cur_instrument_idx();
    }
    else if (first_row_option == 1 && get_num_instruments() >= 2) {
      delete_instrument(get_cur_instrument_idx());
      select_previous_instrument();
    }
  }
  x = MENU_XMARGIN; y += 30;
  x += display_heading("TYPE", x, y) + 30;
  x += display_option("PULSE", x, y, cur_row == 1, instrument->type == PULSE) + 30;
  x += display_option("TRI", x, y, cur_row == 1, instrument->type == TRI) + 30;
  x += display_option("SAW", x, y, cur_row == 1, instrument->type == SAW) + 30;
  x += display_option("SINE", x, y, cur_row == 1, instrument->type == SINE) + 30;
  x += display_option("SAMPLE", x, y, cur_row == 1, instrument->type == SAMPLE) + 30;
  x += display_option("MULTISAMPLE", x, y, cur_row == 1, instrument->type == MULTISAMPLE) + 30;
  BIND_LEFT_ON_ROW(1)
    instrument->type = clamp(instrument->type-1, 0, 5);
  BIND_RIGHT_ON_ROW(1)
    instrument->type = clamp(instrument->type+1, 0, 5);

  x = MENU_XMARGIN; y += 30;
  switch (instrument->type) {
  case PULSE:
    x += display_heading("PULSE WIDTH", x, y) + 30;
    x += display_option(TextFormat("%d%%", (int)(instrument->pulse_width*100)), x, y, cur_row == 2, true) + 30;
    update_float_field(2, &instrument->pulse_width, 0.05, 0.95, 0.01, 0.05);
    adsr_submenu(3, &x, &y);
    break;

  case TRI:
    x += display_heading("NES STYLE?", x, y) + 30;
    x += display_option("YES", x, y, cur_row == 2, instrument->tri_nes_style) + 30;
    x += display_option("NO", x, y, cur_row == 2, !instrument->tri_nes_style) + 30;
    update_bool_field(2, &instrument->tri_nes_style);
    adsr_submenu(3, &x, &y);
    break;

  case SAW:
    x += display_heading("NES STYLE?", x, y) + 30;
    x += display_option("YES", x, y, cur_row == 2, instrument->saw_nes_style) + 30;
    x += display_option("NO", x, y, cur_row == 2, !instrument->saw_nes_style) + 30;
    update_bool_field(2, &instrument->saw_nes_style);
    adsr_submenu(3, &x, &y);
    break;

  case SINE:
    for (int i = 0; i < NUM_HARMONICS; i++) {
      x += display_heading(TextFormat("HARMONIC %d", i+1), x, y) + 30;
      x += display_option(TextFormat("%.2f", instrument->sine_coeffs[i]), x, y, cur_row == 2+i, true) + 30;
      update_float_field(2+i, &instrument->sine_coeffs[i], 0.00, 1.00, 0.01, 0.05);
      x = MENU_XMARGIN; y += 30;
    }
    y -= 30;
    adsr_submenu(2+NUM_HARMONICS, &x, &y);
    break;

  case SAMPLE:
    x += display_heading("SAMPLE FILE", x, y) + 30;
    x += display_text_field(mode_state.sample.path, x, y, cur_row == 2, true) + 30;

    x = MENU_XMARGIN; y += 30;
    x += display_heading("PITCH MODIFIER", x, y) + 30;
    x += display_option(TextFormat("%.2f", mode_state.sample.pitch_modifier), x, y, cur_row == 3, true) + 30;
    update_float_field(3, &mode_state.sample.pitch_modifier, 0.5, 2.0, 0.01, 0.05);

    x = MENU_XMARGIN; y += 30;
    x += display_heading("VOLUME MODIFIER", x, y) + 30;
    x += display_option(TextFormat("%.2f", mode_state.sample.volume_modifier), x, y, cur_row == 4, true) + 30;
    update_float_field(4, &mode_state.sample.volume_modifier, 0.5, 2.0, 0.01, 0.05);

    x = MENU_XMARGIN; y += 30;
    x += display_heading("PLAY CONTINUOUSLY?", x, y) + 30;
    x += display_option("YES", x, y, cur_row == 5, mode_state.sample.play_continuously) + 30;
    x += display_option("NO", x, y, cur_row == 5, !mode_state.sample.play_continuously) + 30;
    update_bool_field(5, &mode_state.sample.play_continuously);

    x = MENU_XMARGIN; y += 30;
    x += display_heading("STOP ON RELEASE?", x, y) + 30;
    x += display_option("YES", x, y, cur_row == 6, mode_state.sample.stop_on_release) + 30;
    x += display_option("NO", x, y, cur_row == 6, !mode_state.sample.stop_on_release) + 30;
    update_bool_field(6, &mode_state.sample.stop_on_release);

    adsr_submenu(7, &x, &y);
    break;

  case MULTISAMPLE:
    multisample_submenu(&x, &y);
    return;  // See below comment
  }

  // The total number of rows under the current instrument type. Note that when
  // the type is MULTISAMPLE, a more complex system is used where both the
  // current multisample entry AND the row of that entry are also stored. Hence the
  // following section is irrelevant.
  int num_rows;
  switch (instrument->type) {
  case PULSE: case TRI: case SAW:
    num_rows = 7;
    break;
  case SINE:
    num_rows = 14;
    break;
  case SAMPLE:
    num_rows = 11;
    break;
  default: break;  // MULTISAMPLE case is already handled above
  }

  if (IsKeyPressed(KEY_UP))
    cur_row = clamp(cur_row-1, -1, num_rows-1);
  if (IsKeyPressed(KEY_DOWN))
    cur_row = clamp(cur_row+1, -1, num_rows-1);
}

void instrument_mode_gui() {
  BeginDrawing();
  ClearBackground((Color){82,64,64,255});
  DrawShadowedTextSE("INSTRUMENT", screen_width-XMARGIN-35, screen_height-YMARGIN-20, 60, WHITE);
  menu();
  EndDrawing();
}
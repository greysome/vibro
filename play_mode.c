#include "play_mode.h"

static void display_note_text_solo_mode() {
  if (!is_any_note_playing()) return;

  int abs_note = get_cur_note() - 1 + get_cur_actual_octave() * 12;
  int scale_degree = mod(abs_note, 12);
  const char *note_text = NULL;

#define match_scale_degree_with_text(n, text)                            \
  if (scale_degree == n)                                                  \
    note_text = TextFormat(text "%d", get_cur_actual_octave() + get_note_octave_modifier(get_cur_note()) + 4)

  match_scale_degree_with_text(0, "C-");
  match_scale_degree_with_text(1, "C#");
  match_scale_degree_with_text(2, "D-");
  match_scale_degree_with_text(3, "D#");
  match_scale_degree_with_text(4, "E-");
  match_scale_degree_with_text(5, "F-");
  match_scale_degree_with_text(6, "F#");
  match_scale_degree_with_text(7, "G-");
  match_scale_degree_with_text(8, "G#");
  match_scale_degree_with_text(9, "A-");
  match_scale_degree_with_text(10, "A#");
  match_scale_degree_with_text(11, "B-");
#undef match_scale_degree_with_text

  DrawShadowedText(note_text, XMARGIN, YMARGIN+20, 40, WHITE);
}

static void display_note_text_chord_mode() {
  int notes_playing[6] = {-1, -1, -1, -1, -1, -1};
  int counter = 0;
  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    NoteState s = get_cur_note_states()[note];
    if (s == PRESSED || s == HELD) {
      notes_playing[counter++] = note;
      if (counter >= 6) break;
    }
  }

  int indices[6] = {0, 0, 0, 0, 0, 0};
  const char *notes_text_1, *notes_text_2;
  const char *note_labels[13] = {"", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  for (int i = 0; i < 6; i++) {
    int note = notes_playing[i];
    if (note == -1)
      indices[i] = 0;
    else
      // I take note+11 instead of note-1 because note+11 > 0, and % behaves like mod
      // only for positive arguments
      indices[i] = (note+11) % 12 + 1;
  }
  notes_text_1 = TextFormat("%s %s %s", note_labels[indices[0]], note_labels[indices[1]], note_labels[indices[2]]);
  notes_text_2 = TextFormat("%s %s %s", note_labels[indices[3]], note_labels[indices[4]], note_labels[indices[5]]);
  DrawShadowedText(notes_text_1, XMARGIN, YMARGIN+20, 30, WHITE);
  DrawShadowedText(notes_text_2, XMARGIN, YMARGIN+50, 30, WHITE);
}

static void display_note_text() {
  if (is_solo_mode())
    display_note_text_solo_mode();
  else if (is_chord_mode())
    display_note_text_chord_mode();
}

static void display_octave_text() {
  int octave = get_global_octave() + get_note_octave_modifier(get_cur_note());
  int local_octave_modifier = get_local_octave_modifier();
  const char* octave_text;
  if (local_octave_modifier == 1)
    octave_text = TextFormat("Octave %d+1", octave + 4);
  else if (local_octave_modifier == -1)
    octave_text = TextFormat("Octave %d-1", octave + 4);
  else 
    octave_text = TextFormat("Octave %d", octave + 4);

  DrawShadowedText(octave_text, XMARGIN, YMARGIN, 20, WHITE);
}

static void display_mode_text() {
  DrawShadowedTextSE("PLAY", screen_width-XMARGIN, screen_height-YMARGIN-20, 60, WHITE);
  if (is_chord_mode())
    DrawShadowedTextSE("CHORDS", screen_width-XMARGIN, screen_height-YMARGIN-3, 20, WHITE);
  else
    DrawShadowedTextSE("SOLO", screen_width-XMARGIN, screen_height-YMARGIN-3, 20, WHITE);
}

static void draw_volume_level() {
  // Computing coordinates
  int x1 = XMARGIN + 170;
  int x2 = XMARGIN + 300;
  int y1 = YMARGIN + 10;
  int y2 = YMARGIN + 50;
  Vector2 v1 = {x1, y2};
  Vector2 v2 = {x2, y2};
  Vector2 v3 = {x2, y1};
  // sustainvol^0.7 is taken so that the gray fill is visible at low volumes
  int x3 = x1 + powf(get_note_vol(), 0.7) * (x2 - x1);
  int y3 = y1 + (1.0 - powf(get_note_vol(), 0.7)) * (y2 - y1);
  Vector2 v1_ = {x1 + 3, y2 + 3};
  Vector2 v2_ = {x2 + 3, y2 + 3};
  Vector2 v3_ = {x2 + 3, y1 + 3};

  // Doing the actual drawing
  // Shadows
  DrawLineEx(v1_, v2_, 3, BLACK);
  DrawLineEx((Vector2){x2 + 3, y2 + 4}, (Vector2){x2 + 3, y1 - 2}, 3, BLACK);
  DrawLineEx(v3_, v1_, 3, BLACK);
  // Actual
  DrawTriangle(v1, (Vector2){x3, y2}, (Vector2){x3, y3}, WHITE);
  DrawLineEx(v1, v2, 3, WHITE);
  DrawLineEx((Vector2){x2, y2 + 1}, (Vector2){x2, y1 - 1}, 3, WHITE);
  DrawLineEx(v3, v1, 3, WHITE);
}

static void draw_straight_line() {
  Vector2 start = {0, screen_height/2};
  Vector2 end = {screen_width, screen_height/2};
  // Shadow
  DrawLineEx((Vector2){0, screen_height/2 + 3}, (Vector2){screen_width, screen_height/2 + 3}, 3, BLACK);
  // Actual
  DrawLineEx(start, end, 3, WHITE);
}

static void draw_wave_sample(Sample *samples) {
  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    if (strnlen(samples[note].path, MAX_STR_LEN) > 0 && !samples[note].is_ready) {
      DrawShadowedTextCenter(TextFormat("Sample not found: %s", samples[note].path), screen_width/2, screen_height/2, 40, WHITE);
      return;
    }
  }

  NoteState *note_states = get_cur_note_states();
  SamplePlaybackState *playback_states = get_sample_playback_states();
  float *actual_vols = get_actual_vols();
  float amplitude;
  float next_amplitude = 0;
  int y, next_y;

  for (int i = 0; i < screen_width; i += 1) {
    amplitude = next_amplitude;
    next_amplitude = 0;

    for (int note = 0; note < NOTETABLE_SIZE; note++) {
      Sample sample = samples[note];
      if (!sample.is_ready || !IsSoundPlaying(sample.sound) || (note_states[note] == RELEASED && sample.stop_on_release)) {
	if (note == 0 && note_states[note] == RELEASED)
	  printf("hi\n");
	continue;
      }
      int frame_counter = playback_states[note].sample_frame_counter;
      float pitch_modifier = playback_states[note].pitch_modifier;

      if (frame_counter + (i+1)*pitch_modifier < sample.num_frames)
	//next_amplitude += sample.data[frame_counter + (int)((i+1)*pitch_modifier)] * sample.volume_modifier * get_note_vol();
	next_amplitude += sample.data[frame_counter + (int)((i+1)*pitch_modifier)] * sample.volume_modifier * actual_vols[note];
    }

    y = screen_height / 2 - 300 * amplitude;
    next_y = screen_height / 2 - 300 * next_amplitude;

    // Shadow
    Vector2 start_shadow = {i+3, y+2};
    Vector2 end_shadow = {i+5, next_y+2};
    DrawLineEx(start_shadow, end_shadow, 2, BLACK);
    // Actual
    Vector2 start = {i, y};
    Vector2 end = {i+2, next_y};
    DrawLineEx(start, end, 2, WHITE);
  }
}

static void draw_wave_periodic() {
  if (is_silent()) {
    draw_straight_line();
    return;
  }

  // TODO phase is a bad word, find a replacement
  float dphase = 1.0 / 30000.0;
  float start_phase = fmodf2((float)GetTime(), WAVESPEED) / WAVESPEED;
  float phases[NOTETABLE_SIZE], next_phases[NOTETABLE_SIZE];
  float amplitude, next_amplitude;
  int y, next_y;
  float *cur_actual_freqs = get_cur_actual_freqs();
  NoteState *note_states = get_cur_note_states();

  for (int note = 0; note < NOTETABLE_SIZE; note++)
    next_phases[note] = start_phase;

  for (int i = 0; i < screen_width; i += 1) {
    for (int note = 0; note < NOTETABLE_SIZE; note++) {
      if (note_states[4]) {}
      phases[note] = next_phases[note];
      next_phases[note] += dphase * cur_actual_freqs[note];
      if (next_phases[note] > 1)
        next_phases[note] -= 1;
    }
    amplitude = get_amplitude(phases);
    next_amplitude = get_amplitude(next_phases);
    y = screen_height / 2 - 300 * amplitude;
    next_y = screen_height / 2 - 300 * next_amplitude;

    // Shadow
    Vector2 start_shadow = {i+3, y+2};
    Vector2 end_shadow = {i+5, next_y+2};
    DrawLineEx(start_shadow, end_shadow, 2, BLACK);
    // Actual
    Vector2 start = {i, y};
    Vector2 end = {i+2, next_y};
    DrawLineEx(start, end, 2, WHITE);
  }
}

void draw_wave() {
  Instrument instrument = *get_cur_instrument();
  if (instrument.type == SAMPLE || instrument.type == MULTISAMPLE)
    draw_wave_sample(instrument.samples);
  else
    draw_wave_periodic();
}

void play_mode_gui() {
  mouse_dx = GetMouseDelta().x;
  mouse_dy = GetMouseDelta().y;
  if (fabs(mouse_dx) >= fabs(mouse_dy)) mouse_dy = 0;
  else mouse_dx = 0;

  if (IsKeyPressed(KEY_LEFT_CONTROL)) {
    toggle_chord_mode();
    reset_freq_modifiers();
  }

  if (IsKeyPressed(KEY_GRAVE)) {
    if (!is_recording)
      tinywav_open_write(&tw, 1, SAMPLE_RATE, TW_INT16, TW_INLINE, RECORD_FILE);
    else
      tinywav_close_write(&tw);
    is_recording = !is_recording;
  }

  update_global_octave();
  // Local/global octave needs to be updated BEFORE note state.
  // This is because a change in octave will cause change in note state,
  // even if the same key is being held.
  update_local_octave_modifier();
  update_note_state();

  update_pitch_bend();
  update_autogliss();
  update_gliss();
  update_dive();
  update_vib();

  update_note_vol();
  apply_adsr();

  Instrument instrument = *get_cur_instrument();
  if (instrument.type == SAMPLE) {
    for (int note = 0; note < NOTETABLE_SIZE; note++)
      handle_sample_playback(note, instrument.samples[note]);
  }
  else if (instrument.type == MULTISAMPLE) {
    if (is_solo_mode()) {
      int note = get_cur_note_or_prev();
      if (note != NIL)
	handle_sample_playback(note, instrument.samples[note]);
    }
    else if (is_chord_mode()) {
      for (int note = 0; note < NOTETABLE_SIZE; note++)
	handle_sample_playback(note, instrument.samples[note]);
    }
  }

  BeginDrawing();
  ClearBackground((Color){64,82,74,255});
  draw_wave();
  display_octave_text();
  display_note_text();
  display_mode_text();
  DrawShadowedTextCenter(instrument.name, screen_width/2, screen_height-YMARGIN-20, 30, WHITE);
  draw_volume_level();
  EndDrawing();
}
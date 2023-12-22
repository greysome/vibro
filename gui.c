/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "gui.h"

void display_note_text_solo_mode() {
  if (!is_any_note_playing()) return;

  int abs_note = get_cur_note() - 1 + get_cur_actual_octave() * 12;
  int scale_degree = mod(abs_note, 12);
  const char *note_text;

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

  DrawText(note_text, XMARGIN+3, YMARGIN+23, FONTSIZE * 2, BLACK);
  DrawText(note_text, XMARGIN, YMARGIN+20, FONTSIZE * 2, WHITE);
}

void display_note_text_chord_mode() {
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
  DrawText(notes_text_1, XMARGIN+3, YMARGIN+20, FONTSIZE*1.5, BLACK);
  DrawText(notes_text_1, XMARGIN, YMARGIN+20, FONTSIZE*1.5, WHITE);
  DrawText(notes_text_2, XMARGIN+3, YMARGIN+50, FONTSIZE*1.5, BLACK);
  DrawText(notes_text_2, XMARGIN, YMARGIN+50, FONTSIZE*1.5, WHITE);
}

void display_note_text() {
  if (is_solo_mode())
    display_note_text_solo_mode();
  else if (is_chord_mode())
    display_note_text_chord_mode();
}

void display_octave_text() {
  int octave = get_global_octave() + get_note_octave_modifier(get_cur_note());
  int local_octave_modifier = get_local_octave_modifier();
  const char* octave_text;
  if (local_octave_modifier == 1)
    octave_text = TextFormat("Octave %d+1", octave + 4);
  else if (local_octave_modifier == -1)
    octave_text = TextFormat("Octave %d-1", octave + 4);
  else 
    octave_text = TextFormat("Octave %d", octave + 4);

  DrawText(octave_text, XMARGIN+3, YMARGIN+3, FONTSIZE, BLACK);
  DrawText(octave_text, XMARGIN, YMARGIN, FONTSIZE, WHITE);
}

static void DrawTextSE(char *text, int x, int y, int font_size, Color color) {
  Vector2 text_dimens = MeasureTextEx(GetFontDefault(), text, font_size, 0);
  DrawText(text, x - text_dimens.x, y - text_dimens.y, font_size, color);
}

void display_mode_text() {
  DrawTextSE("PLAY", screen_width-XMARGIN+5, screen_height-YMARGIN-25, FONTSIZE*3, BLACK);
  DrawTextSE("PLAY", screen_width-XMARGIN, screen_height-YMARGIN-30, FONTSIZE*3, WHITE);
  if (is_chord_mode()) {
    DrawTextSE("CHORDS", screen_width-XMARGIN+3, screen_height-YMARGIN-7, FONTSIZE, BLACK);
    DrawTextSE("CHORDS", screen_width-XMARGIN, screen_height-YMARGIN-10, FONTSIZE, WHITE);
  }
  else {
    DrawTextSE("SOLO", screen_width-XMARGIN+3, screen_height-YMARGIN-7, FONTSIZE, BLACK);
    DrawTextSE("SOLO", screen_width-XMARGIN, screen_height-YMARGIN-10, FONTSIZE, WHITE);
  }
}

void draw_volume_level() {
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

void draw_wave() {
  if (is_silent()) {
    Vector2 start = {0, screen_height / 2};
    Vector2 end = {screen_width, screen_height / 2};
    // Shadow
    DrawLineEx((Vector2){0, screen_height / 2 + 3},
               (Vector2){screen_width, screen_height / 2 + 3}, 3, BLACK);
    // Actual
    DrawLineEx(start, end, 3, WHITE);
    return;
  }

  // TODO phase is a bad word, find a replacement
  float dphase = 1.0 / 30000.0;
  float start_phase = fmodf2((float)GetTime(), WAVESPEED) / WAVESPEED;
  float phases[NOTETABLE_SIZE], next_phases[NOTETABLE_SIZE];
  float amplitude, next_amplitude;
  int y, next_y;
  float *cur_actual_freqs = get_cur_actual_freqs();

  for (int note = 0; note < NOTETABLE_SIZE; note++)
    next_phases[note] = start_phase;
  
  for (int i = 0; i < screen_width; i += 1) {
    for (int note = 0; note < NOTETABLE_SIZE; note++) {
      phases[note] = next_phases[note];
      next_phases[note] += dphase * cur_actual_freqs[note];
      if (next_phases[note] > 1) next_phases[note] -= 1;
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
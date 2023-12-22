/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "gui.h"

void display_note_text() {
  int abs_note = get_cur_note() + get_cur_actual_octave() * 12;
  int scale_degree = mod(abs_note, 12);
  const char *note_text;

#define match_scale_degree_with_text(n, text)                            \
  if (scale_degree == n)                                                  \
    note_text = TextFormat(text "%d", get_cur_actual_octave() + get_note_octave_modifier() + 4)

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

  if (is_note_down()) {
    DrawText(note_text, XMARGIN + 23, YMARGIN + 23, FONTSIZE * 2, BLACK);
    DrawText(note_text, XMARGIN + 20, YMARGIN + 20, FONTSIZE * 2, WHITE);
  }
}

void display_octave_text() {
  int octave = get_global_octave() + get_note_octave_modifier();
  int local_octave_modifier = get_local_octave_modifier();
  const char* octave_text;
  if (local_octave_modifier == 1)
    octave_text = TextFormat("Octave %d+1", octave + 4);
  else if (local_octave_modifier == -1)
    octave_text = TextFormat("Octave %d-1", octave + 4);
  else 
    octave_text = TextFormat("Octave %d", octave + 4);

  DrawText(octave_text, XMARGIN + 23, YMARGIN + 3, FONTSIZE, BLACK);
  DrawText(octave_text, XMARGIN + 20, YMARGIN, FONTSIZE, WHITE);
}

void draw_volume_level() {
  // Computing coordinates
  int x1 = XMARGIN + 130;
  int x2 = XMARGIN + 300;
  int y1 = YMARGIN + 10;
  int y2 = YMARGIN + 50;
  Vector2 v1 = {x1, y2};
  Vector2 v2 = {x2, y2};
  Vector2 v3 = {x2, y1};
  // sustainvol^0.7 is taken so that the gray fill is visible at low volumes
  int x3 = x1 + powf(get_sustain_vol(), 0.7) * (x2 - x1);
  int y3 = y1 + (1.0 - powf(get_sustain_vol(), 0.7)) * (y2 - y1);
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
  if (get_actual_vol() == 0) {
    Vector2 start = {0, screenheight / 2};
    Vector2 end = {screenwidth, screenheight / 2};
    // Shadow
    DrawLineEx((Vector2){0, screenheight / 2 + 3},
               (Vector2){screenwidth, screenheight / 2 + 3}, 3, BLACK);
    // Actual
    DrawLineEx(start, end, 3, WHITE);
    return;
  }

  float dphase = get_cur_actual_freq() / WAVEXSCALE;
  float phase = fmodf2((float)GetTime(), WAVESPEED) / WAVESPEED;
  int y, y_;
  for (int i = 0; i < screenwidth; i += 2) {
    phase += dphase;
    float nextphase = phase + dphase;
    if (nextphase >= 1)
      nextphase -= 1;
    if (wavetype == TRI) {
      y = screenheight / 2 - 100 * get_actual_vol() * nes_tri(phase);
      y_ = screenheight / 2 - 100 * get_actual_vol() * nes_tri(nextphase);
    } else if (wavetype == SAW) {
      y = screenheight / 2 - 100 * get_actual_vol() * nes_saw(phase);
      y_ = screenheight / 2 - 100 * get_actual_vol() * nes_saw(nextphase);
    } else if (wavetype == PULSE) {
      y = screenheight / 2 - 100 * get_actual_vol() * nes_pulse(phase);
      y_ = screenheight / 2 - 100 * get_actual_vol() * nes_pulse(nextphase);
    } else if (wavetype == SINE) {
      y = screenheight / 2 - 100 * get_actual_vol() * add_synthesise(phase);
      y_ = screenheight / 2 - 100 * get_actual_vol() * add_synthesise(nextphase);
    }
    Vector2 start = {i, y};
    Vector2 end = {i + 2, y_};
    // Shadow
    DrawLineEx((Vector2){i + 3, y + 2}, (Vector2){i + 5, y_ + 2}, 2, BLACK);
    // Actual
    DrawLineEx(start, end, 2, WHITE);
    if (phase >= 1)
      phase = fmodf(phase, 1.0);
  }
}

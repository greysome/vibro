/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "volume.h"

// Sustain volume as a proportion of MAXVOL.
// This can be controlled by the number keys on the keyboard,
// or by left-and-right mouse movement.
static float note_vol = 0.5;
// Updates frame-by-frame according to ADSR envelope
static float actual_vol = 0.0;

void update_sustain_vol() {
  note_vol += clamp(mousedx * 0.001, -0.01, 0.01);
  note_vol = clamp(note_vol, 0, 1);

#define match_key_to_sustain_vol(key, vol) \
  if (IsKeyDown(key)) { note_vol = vol; }

  match_key_to_sustain_vol(KEY_ONE, 0.1);
  match_key_to_sustain_vol(KEY_TWO, 0.2);
  match_key_to_sustain_vol(KEY_THREE, 0.3);
  match_key_to_sustain_vol(KEY_FOUR, 0.4);
  match_key_to_sustain_vol(KEY_FIVE, 0.5);
  match_key_to_sustain_vol(KEY_SIX, 0.6);
  match_key_to_sustain_vol(KEY_SEVEN, 0.7);
  match_key_to_sustain_vol(KEY_EIGHT, 0.8);
  match_key_to_sustain_vol(KEY_NINE, 0.9);
  match_key_to_sustain_vol(KEY_ZERO, 1.0);
#undef match_key_to_sustain_vol
}

void apply_adsr() {
  static ADSRState adsr_state = RELEASE;
  static int attack_frames = 1;
  static int decay_frames = 10;
  static float sustain_vol = 0.2; // A proportion of note_vol
  static int release_frames = 5;
  static float release_peak;  // Value of actualvol right when release starts

  if (get_cur_note_state() == PRESSED) {
    adsr_state = ATTACK;
    // It is possible to set this to 0 instead, but it results in unpleasant
    // popping noises when switching notes repeatedly
    actual_vol = note_vol / (float)attack_frames;
  }
  else if (get_cur_note_state() == HELD) {
    if (adsr_state == ATTACK) {
      actual_vol += note_vol / (float)attack_frames;
      if (actual_vol >= note_vol) {
        adsr_state = DECAY;
        actual_vol = note_vol;
      }
    }
    else if (adsr_state == DECAY) {
      actual_vol -= (note_vol - sustain_vol) / (float)decay_frames;
      if (actual_vol <= sustain_vol) {
        adsr_state = SUSTAIN;
        actual_vol = sustain_vol;
      }
    }
  }
  else if (get_cur_note_state() == RELEASED) {
    adsr_state = RELEASE;
    release_peak = actual_vol;
  }
  else if (get_cur_note_state() == IDLE) {
    if (actual_vol > 0)
      actual_vol -= release_peak / (float)release_frames;
    else
      actual_vol = 0;
  }
}

float get_sustain_vol() {
  return note_vol;
}

float get_actual_vol() {
  return actual_vol;
}
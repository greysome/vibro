/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef FREQ
#define FREQ

#include "raylib.h"
#include "globals.h"
#include "debug.h"
#include "keys.h"

// Moving up a semitone increases the Hz by this factor
#define SEMITONE 1.05946
// In Hz
#define C4 261.6

float get_note_freq(int note, int octave);
float get_cur_note_freq();
float get_prev_note_freq();
float get_actual_freq(int note, int octave);
void update_pitch_bend();
void update_gliss();
void update_effects();
void update_vib();
void update_autogliss();
float get_cur_actual_freq();

#endif
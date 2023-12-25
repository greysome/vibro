#ifndef FREQ
#define FREQ

#include "raylib.h"
#include "globals.h"
#include "debug.h"
#include "note.h"
#include "util.h"

// Moving up a semitone increases the Hz by this factor
#define SEMITONE 1.05946
// In Hz
#define C4 261.6

float get_note_freq(int note, int octave);
float get_actual_freq(int note, int octave);
void update_pitch_bend();
void update_gliss();
void update_effects();
void update_vib();
void update_autogliss();
float *get_cur_actual_freqs();
void reset_freq_modifiers();

#endif
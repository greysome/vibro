#ifndef FREQ
#define FREQ

#include "raylib.h"
#include "note.h"
#include "util.h"

#define SEMITONE 1.05946 // Moving up a semitone scales the frequency by this factor
#define C4 261.6 // In Hz

// Pitch modifiers refer to the following expressive controls:
// pitch bend, gliss, dive, vibrato, autogliss;
// which can alter the pitch of the currently played note.

float get_note_freq(int note, int octave); /* Get frequency of note at octave, without any pitch modifiers. */
 /* Get frequency of note at octave, with pitch modifiers.
    NOTE: this does not account for autogliss. */
float get_actual_freq(int note, int octave);
void update_pitch_bend();
bool is_autoglissing();
void update_gliss();
void update_dive();
void update_vib();
void update_autogliss();
/* Returns a table of frequencies for each note in the notetable, accounting for pitch modifiers.
   Unlike get_actual_freq(), this accounts for autogliss.
   NOTE: ths can be used in solo mode, but you should only trust the entry given by get_cur_note(). */
float *get_cur_actual_freqs();
float get_bend_modifier();
float get_gliss_modifier();
float get_vib_modifier();
float get_dive_modifier();
float get_autogliss_modifier();
void reset_freq_modifiers();

#endif
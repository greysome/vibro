#ifndef OCTAVE
#define OCTAVE

#include "raylib.h"
#include "globals.h"
#include "util.h"

#define MIN_OCTAVE -4
#define MAX_OCTAVE 2

void update_global_octave();
void update_local_octave_modifier();
int get_global_octave();
int get_local_octave_modifier();
int get_note_octave_modifier(int note);
int get_cur_actual_octave();
int get_prev_actual_octave();

#endif
/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef OCTAVE
#define OCTAVE

#include "raylib.h"
#include "debug.h"
#include "globals.h"

void update_global_octave();
void update_local_octave_modifier();
int get_global_octave();
int get_local_octave_modifier();
int get_note_octave_modifier();
int get_cur_actual_octave();
int get_prev_actual_octave();

#endif
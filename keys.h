/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef KEYS
#define KEYS

#include <string.h>
#include "raylib.h"
#include "globals.h"
#include "octave.h"

typedef enum {
  PRESSED, HELD, RELEASED, IDLE
} NoteState;

bool is_chord_mode();
void update_note_state();

int get_cur_note();
int get_prev_note();
NoteState get_prev_note_state();
NoteState get_cur_note_state();
void kill_note();
void no_attack();
bool is_legato();
bool is_note_down();

int *get_cur_notes();
NoteState *get_cur_note_states();
void update_note_states();

#endif
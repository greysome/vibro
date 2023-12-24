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

#define KEYTABLE_SIZE 33
#define NOT_HELD -100
#define MAX_VOICES 6
// Only 6 notes can be played at once, but internally more notes than
// that may have to be handled, e.g. if a chord of 6 notes was just
// released and user immediately pressed another chord of 6 notes
#define MAX_NOTE_ENTRIES KEYTABLE_SIZE

typedef enum {
  PRESSED, HELD, RELEASED, STILLRELEASED, IDLE
} NoteState;

/* Common functions */
int *get_cur_notes();
NoteState *get_cur_note_states();
int get_prev_note();
NoteState get_prev_note_state();
bool is_chord_mode();
bool is_solo_mode();
void toggle_chord_mode();
bool is_note_down();
bool is_note_pressed();
void kill_note(int note);
void kill_notes();

/* Specific to solo mode */
int get_cur_note();
NoteState get_cur_note_state();
bool is_legato();
void no_attack();

void update_note_state();

#endif
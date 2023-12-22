/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "keys.h"

#define KEYTABLE_SIZE 33
// The keytable keeps track of which keys are pressed. It is
// used to determine which note to play out, in the case that
// multiple keys were pressed.
static short keytable[KEYTABLE_SIZE];
// keytable of previous frame. Used to determine which notes
// are newly pressed or released
static short keytable_prev[KEYTABLE_SIZE];

#define MAX_VOICES 6
// Only 6 notes can be played at once, but internally more notes than
// that may have to be handled, e.g. if a chord of 6 notes was just
// released and user immediately pressed another chord of 6 notes
#define MAX_NOTE_ENTRIES 10 * MAX_VOICES
#define NOT_HELD -100
static bool chord_mode = true;

// VARIABLES FOR SOLO MODE
static NoteState cur_note_state;
static NoteState prev_note_state;
static int cur_note;
static int prev_note;

// VARIABLES FOR CHORD MODE
static NoteState cur_note_states[MAX_NOTE_ENTRIES];
static NoteState prev_note_state;
// The number of semitones above the C at the current octave.
// Only changes when a new note is pressed.
static int cur_notes[MAX_NOTE_ENTRIES];

/** *****************************/
/** COMMON FUNCTIONS            */
/** *****************************/

bool is_chord_mode() { return chord_mode; }

static void update_keytables() {
  for (int i = 0; i < KEYTABLE_SIZE; i++)
    keytable_prev[i] = keytable[i];

#define map_key_to_keytable_entry(key, note) keytable[note] = IsKeyDown(key)
#define map_keys_to_keytable_entry(key1, key2, note) keytable[note] = IsKeyDown(key1) || IsKeyDown(key2)

  map_key_to_keytable_entry(KEY_A, 0);
  map_key_to_keytable_entry(KEY_Z, 1);
  map_key_to_keytable_entry(KEY_S, 2);
  map_key_to_keytable_entry(KEY_X, 3);
  map_key_to_keytable_entry(KEY_D, 4);
  map_key_to_keytable_entry(KEY_C, 5);
  map_key_to_keytable_entry(KEY_V, 6);
  map_key_to_keytable_entry(KEY_G, 7);
  map_key_to_keytable_entry(KEY_B, 8);
  map_key_to_keytable_entry(KEY_H, 9);
  map_key_to_keytable_entry(KEY_N, 10);
  map_key_to_keytable_entry(KEY_J, 11);

  if (chord_mode) {
    map_keys_to_keytable_entry(KEY_M, KEY_ONE, 12);
    map_keys_to_keytable_entry(KEY_COMMA, KEY_Q, 13);
    map_keys_to_keytable_entry(KEY_L, KEY_TWO, 14);
    map_keys_to_keytable_entry(KEY_PERIOD, KEY_W, 15);
    map_keys_to_keytable_entry(KEY_SEMICOLON, KEY_THREE, 16);
    map_keys_to_keytable_entry(KEY_SLASH, KEY_E, 17);
  }
  else {
    map_key_to_keytable_entry(KEY_M, 12);
    map_key_to_keytable_entry(KEY_COMMA, 13);
    map_key_to_keytable_entry(KEY_L, 14);
    map_key_to_keytable_entry(KEY_PERIOD, 15);
    map_key_to_keytable_entry(KEY_SEMICOLON, 16);
    map_key_to_keytable_entry(KEY_SLASH, 17);
  }

  if (chord_mode) {
    map_key_to_keytable_entry(KEY_R, 18);
    map_key_to_keytable_entry(KEY_FIVE, 19);
    map_key_to_keytable_entry(KEY_T, 20);
    map_key_to_keytable_entry(KEY_SIX, 21);
    map_key_to_keytable_entry(KEY_Y, 22);
    map_key_to_keytable_entry(KEY_SEVEN, 23);
    map_key_to_keytable_entry(KEY_U, 24);
    map_key_to_keytable_entry(KEY_I, 25);
    map_key_to_keytable_entry(KEY_NINE, 26);
    map_key_to_keytable_entry(KEY_O, 27);
    map_key_to_keytable_entry(KEY_ZERO, 28);
    map_key_to_keytable_entry(KEY_P, 29);
    map_key_to_keytable_entry(KEY_LEFT_BRACKET, 30);
    map_key_to_keytable_entry(KEY_EQUAL, 31);
    map_key_to_keytable_entry(KEY_RIGHT_BRACKET, 32);
  }
#undef map_key_to_keytable_entry
}

/** *****************************/
/** SOLO MODE                   */
/** *****************************/

int get_cur_note() { return cur_notes[0]; }
int get_prev_note() { return prev_note; }
NoteState get_prev_note_state() { return prev_note_state; }
NoteState get_cur_note_state() { return cur_note_states[0]; }

bool is_legato() {
  return (prev_note_state == HELD) && (cur_note_states[0] == PRESSED);
}

bool is_note_down() {
  for (int i = 0; i < MAX_VOICES; i++) {
    NoteState s = cur_note_states[i];
    if (i == PRESSED || i == HELD)
      return true;
  }
  return false;
}

void kill_note() {
  if (is_note_down()) {
    prev_note_state = cur_note_state;
    cur_note_state = RELEASED;
  }
}

void no_attack() {
  if (is_note_down())
    cur_note_state = HELD;
}

void update_note_state() {
  update_keytables();
  prev_note = cur_note;
  prev_note_state = cur_note_state;
  // Any change to the keytable?
  bool changed = 0;
  // A note that has remained held, to use as current note in case
  // some other note is released. Otherwise, held_note = -1 means
  // no note is being held.
  static int held_note = -1;

  held_note = NOT_HELD;
  for (int i = 0; i < KEYTABLE_SIZE; i++) {
    // Note is still being held
    if (keytable_prev[i] && keytable[i])
      held_note = i - 1;  // Subtract 1 because the first keytable entry is actually a B
    // Note is newly pressed
    else if (!keytable_prev[i] && keytable[i]) {
      changed = 1;
      held_note = i - 1;
      break;  // Break because a newly pressed note is automatically the current note
    }
    // Note is just released
    else if (keytable_prev[i] && !keytable[i]) {
      changed = 1;
    }
  }
  // Also account for octave changes even when the same keys are being held
  changed |= get_prev_actual_octave() != get_cur_actual_octave();

  // If the held note is currently being played at the same octave, there is no change
  // even if other notes were released.
  if (get_prev_actual_octave() == get_cur_actual_octave() && is_note_down() && held_note == get_cur_note())
    changed = 0;

  if (changed) {
    if (held_note == NOT_HELD)
      cur_note_state = RELEASED;
    else {
      cur_note_state = PRESSED;
      cur_note = held_note;
    }
  }
  else
    cur_note_state = (held_note == NOT_HELD) ? IDLE : HELD;
}

/** *****************************/
/** CHORD MODE                  */
/** *****************************/

int *get_cur_notes() { return cur_notes; }
NoteState *get_cur_note_states() { return cur_note_states; }

void update_note_states() {
  memset(cur_notes, NOT_HELD, MAX_NOTE_ENTRIES * sizeof(int));
  memset(cur_note_states, IDLE, MAX_NOTE_ENTRIES * sizeof(NoteState));

  update_keytables();
  int j = 0;

  for (int i = 0; i < KEYTABLE_SIZE; i++) {
    if (keytable_prev[i] && keytable[i]) {
      if (j > MAX_VOICES) continue;
      cur_notes[j] = i;
      cur_note_states[j++] = HELD;
    }
    else if (!keytable_prev[i] && keytable[i]) {
      if (j > MAX_VOICES) continue;
      cur_notes[j] = i;
      cur_note_states[j++] = PRESSED;
    }
    else if (keytable_prev[i] && !keytable[i]) {
      if (j > 10 * MAX_VOICES) break;
      cur_notes[j] = i;
      cur_note_states[j++] = RELEASED;
    }
  }

  printf("cur notes: ");
  for (int i = 0; i < 2 * MAX_VOICES; i++)
    printf("%d ", cur_notes[i]);
  printf("\n");

  printf("cur note states: ");
  for (int i = 0; i < 2 * MAX_VOICES; i++)
    printf("%d ", cur_note_states[i]);
  printf("\n");
}
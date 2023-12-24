/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "keys.h"
#include "raylib.h"

// The keytable keeps track of which keys are pressed. It is
// used to determine which note to play out, in the case that
// multiple keys were pressed.
static short keytable[KEYTABLE_SIZE];
// keytable of previous frame. Used to determine which notes
// are newly pressed or released
static short keytable_prev[KEYTABLE_SIZE];

static bool chord_mode = false;

static NoteState cur_note_states[KEYTABLE_SIZE];
// ONLY FOR SOLO MODE
static NoteState prev_note_state;
static int prev_note;

/** *****************************/
/** COMMON FUNCTIONS            */
/** *****************************/

NoteState *get_cur_note_states() { return cur_note_states; }
int get_prev_note() { return prev_note; }
NoteState get_prev_note_state() { return prev_note_state; }

bool is_chord_mode() { return chord_mode; }
bool is_solo_mode() { return !chord_mode; }
void toggle_chord_mode() {
  chord_mode = !chord_mode;
  kill_notes();
}

bool is_note_down() {
  for (int i = 0; i < KEYTABLE_SIZE; i++) {
    NoteState s = cur_note_states[i];
    if (s == PRESSED || s == HELD)
      return true;
  }
  return false;
}

bool is_note_pressed() {
  for (int note = 0; note < KEYTABLE_SIZE; note++) {
    NoteState s = cur_note_states[note];
    if (s == PRESSED)
      return true;
  }
  return false;
}

void kill_note(int note) {
  if (is_solo_mode()) {
    prev_note_state = cur_note_states[note];
    cur_note_states[note] = IDLE;
  }
  else {
    cur_note_states[note] = IDLE;
  }
}

void kill_notes() {
  for (int note = 0; note < KEYTABLE_SIZE; note++) {
    kill_note(note);
  }
}

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

int get_cur_note() {
  for (int i = 0; i < KEYTABLE_SIZE; i++)
    if (cur_note_states[i] == PRESSED || cur_note_states[i] == HELD)
      return i;
  return -1;
}

NoteState get_cur_note_state() {
  int note = get_cur_note();
  if (note == -1) note = prev_note;
  return cur_note_states[note];
}

bool is_legato() {
  return (prev_note_state == HELD) && (get_cur_note_state() == PRESSED);
}

void no_attack() {
  int note = get_cur_note();
  if (note != -1 && cur_note_states[note] == PRESSED)
    cur_note_states[note] = HELD;
}

void update_note_state_solo_mode() {
  update_keytables();
  if (get_cur_note() != -1)
    prev_note = get_cur_note();
  prev_note_state = get_cur_note_state();

  int pressed_note = NOT_HELD;
  int held_note = NOT_HELD;
  bool any_released = false;

  for (int note = 0; note < KEYTABLE_SIZE; note++) {
    if (keytable_prev[note] && keytable[note])  // Note is still being held
      held_note = note;
    else if (!keytable_prev[note] && keytable[note]) {  // Note is newly pressed
      pressed_note = note;
      break;  // Break because a newly pressed note is automatically the current note
    }
    else if (keytable_prev[note] && !keytable[note]) {  // Note is just released
      any_released = true;
      cur_note_states[note] = RELEASED;
    }
    else
      cur_note_states[note] = STILLRELEASED;
  }

  // If octave is changed while note is held, pretend the held note is newly pressed
  if (get_prev_actual_octave() != get_cur_actual_octave() && is_note_down()) {
    pressed_note = get_cur_note();
    held_note = NOT_HELD;
  }

  // A newly pressed note is automatically the new current note
  if (pressed_note != NOT_HELD) {
    cur_note_states[pressed_note] = PRESSED;
    for (int note = 0; note < KEYTABLE_SIZE; note++)
      if (note != pressed_note && cur_note_states[note] != RELEASED)
	cur_note_states[note] = IDLE;
  }
  // If no new pressed or released notes but notes are still held, then stick with currently held note
  else if (pressed_note == NOT_HELD && held_note != NOT_HELD && !any_released) { 
    cur_note_states[get_cur_note()] = HELD;
    for (int note = 0; note < KEYTABLE_SIZE; note++)
      if (note != get_cur_note() && cur_note_states[note] != RELEASED)
	cur_note_states[note] = IDLE;
  }
  // If no new pressed notes but a note was released, then use the most recent held note and pretend it was newly pressed
  // If the held note is the note already playing, then don't attack it (i.e. set its note state to HELD)
  else if (pressed_note == NOT_HELD && held_note != NOT_HELD && any_released) {
    cur_note_states[held_note] = (held_note == prev_note) ? HELD : PRESSED;
    for (int note = 0; note < KEYTABLE_SIZE; note++)
      if (note != held_note && cur_note_states[note] != RELEASED)
	cur_note_states[note] = IDLE;
  }
}


/** *****************************/
/** CHORD MODE                  */
/** *****************************/

void update_note_state_chord_mode() {
  update_keytables();
  for (int note = 0; note < KEYTABLE_SIZE; note++) {
    if (get_cur_note_states()[note] == RELEASED)
      cur_note_states[note] = STILLRELEASED;
    else if (keytable_prev[note] && keytable[note])
      cur_note_states[note] = HELD;
    else if (!keytable_prev[note] && keytable[note])
      cur_note_states[note] = PRESSED;
    else if (keytable_prev[note] && !keytable[note])
      cur_note_states[note] = RELEASED;
  }
}

void update_note_state() {
  if (is_chord_mode()) update_note_state_chord_mode();
  else update_note_state_solo_mode();
}
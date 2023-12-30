#include "note.h"

// Chord mode allows multiple notes to be played at once
// On the flipside, solo mode only allows one note at a time, but it has more
// expressive features like autoglissing.
static bool chord_mode = false;

// Each note is bound to a set of keys. If any of those keys are pressed, then
// the corresponding entry in the notetable is true.
// The notetable is used to determine which note(s) to play in the output (a single
// note in solo mode, possibly multiple notes in chord mode).
static bool notetable[NOTETABLE_SIZE];
// The notetable for the previous frame. Used to determine which notes are newly
// pressed or released.
static bool notetable_prev[NOTETABLE_SIZE];

// The state of each note
static NoteState cur_note_states[NOTETABLE_SIZE];
// (SOLO MODE)
// The state of the currently played note in the previous frame. Used to
// determine whether to execute autogliss.
static NoteState prev_note_state;
// (SOLO MODE)
// The note played in the previous frame.
static int prev_note = NIL;

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

bool is_any_note_playing() {
  for (int i = 0; i < NOTETABLE_SIZE; i++) {
    NoteState s = cur_note_states[i];
    if (s == PRESSED || s == HELD)
      return true;
  }
  return false;
}

bool is_any_note_pressed() {
  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    NoteState s = cur_note_states[note];
    if (s == PRESSED)
      return true;
  }
  return false;
}

void kill_note(int note) {
  if (is_solo_mode()) {
    if (note == get_cur_note())
      prev_note_state = cur_note_states[note];
    cur_note_states[note] = IDLE;
  }
  else
    cur_note_states[note] = IDLE;
}

void kill_notes() {
  for (int note = 0; note < NOTETABLE_SIZE; note++)
    kill_note(note);
}

static void update_notetables() {
  for (int i = 0; i < NOTETABLE_SIZE; i++)
    notetable_prev[i] = notetable[i];

#define map_key_to_notetable_entry(key, note) notetable[note] = IsKeyDown(key)
#define map_keys_to_notetable_entry(key1, key2, note) notetable[note] = IsKeyDown(key1) || IsKeyDown(key2)

  map_key_to_notetable_entry(KEY_A, 0);
  map_key_to_notetable_entry(KEY_Z, 1);
  map_key_to_notetable_entry(KEY_S, 2);
  map_key_to_notetable_entry(KEY_X, 3);
  map_key_to_notetable_entry(KEY_D, 4);
  map_key_to_notetable_entry(KEY_C, 5);
  map_key_to_notetable_entry(KEY_V, 6);
  map_key_to_notetable_entry(KEY_G, 7);
  map_key_to_notetable_entry(KEY_B, 8);
  map_key_to_notetable_entry(KEY_H, 9);
  map_key_to_notetable_entry(KEY_N, 10);
  map_key_to_notetable_entry(KEY_J, 11);

  if (chord_mode) {
    map_keys_to_notetable_entry(KEY_M, KEY_ONE, 12);
    map_keys_to_notetable_entry(KEY_COMMA, KEY_Q, 13);
    map_keys_to_notetable_entry(KEY_L, KEY_TWO, 14);
    map_keys_to_notetable_entry(KEY_PERIOD, KEY_W, 15);
    map_keys_to_notetable_entry(KEY_SEMICOLON, KEY_THREE, 16);
    map_keys_to_notetable_entry(KEY_SLASH, KEY_E, 17);
    map_keys_to_notetable_entry(KEY_APOSTROPHE, KEY_R, 18);
  }
  else {
    map_key_to_notetable_entry(KEY_M, 12);
    map_key_to_notetable_entry(KEY_COMMA, 13);
    map_key_to_notetable_entry(KEY_L, 14);
    map_key_to_notetable_entry(KEY_PERIOD, 15);
    map_key_to_notetable_entry(KEY_SEMICOLON, 16);
    map_key_to_notetable_entry(KEY_SLASH, 17);
    map_key_to_notetable_entry(KEY_R, 18);
  }

  if (chord_mode) {
    map_key_to_notetable_entry(KEY_FIVE, 19);
    map_key_to_notetable_entry(KEY_T, 20);
    map_key_to_notetable_entry(KEY_SIX, 21);
    map_key_to_notetable_entry(KEY_Y, 22);
    map_key_to_notetable_entry(KEY_SEVEN, 23);
    map_key_to_notetable_entry(KEY_U, 24);
    map_key_to_notetable_entry(KEY_I, 25);
    map_key_to_notetable_entry(KEY_NINE, 26);
    map_key_to_notetable_entry(KEY_O, 27);
    map_key_to_notetable_entry(KEY_ZERO, 28);
    map_key_to_notetable_entry(KEY_P, 29);
    map_key_to_notetable_entry(KEY_LEFT_BRACKET, 30);
    map_key_to_notetable_entry(KEY_EQUAL, 31);
    map_key_to_notetable_entry(KEY_RIGHT_BRACKET, 32);
  }
#undef map_key_to_notetable_entry
}

/** *****************************/
/** SOLO MODE                   */
/** *****************************/

int get_cur_note() {
  for (int i = 0; i < NOTETABLE_SIZE; i++)
    if (cur_note_states[i] == PRESSED || cur_note_states[i] == HELD)
      return i;
  return NIL;
}

int get_cur_note_or_prev() {
  int note = get_cur_note();
  if (note == NIL)
    note = get_prev_note();
  return note;
}

NoteState get_cur_note_state() {
  int note = get_cur_note();
  return note == NIL ? IDLE : cur_note_states[note];
}

bool is_legato() {
  return (prev_note_state == HELD) && (get_cur_note_state() == PRESSED);
}

void no_attack() {
  int note = get_cur_note();
  if (note != NIL && cur_note_states[note] == PRESSED)
    cur_note_states[note] = HELD;
}

static void update_note_state_in_solo_mode() {
  update_notetables();
  if (get_cur_note() != NIL)
    prev_note = get_cur_note();
  prev_note_state = get_cur_note_state();

  int pressed_note = NIL;
  int held_note = NIL;
  bool any_released = false;

  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    if (notetable_prev[note] && notetable[note])  // Note is still being held
      held_note = note;
    else if (!notetable_prev[note] && notetable[note]) {  // Note is newly pressed
      pressed_note = note;
      break;  // Break because a newly pressed note is automatically the current note
    }
    else if (notetable_prev[note] && !notetable[note]) {  // Note is just released
      any_released = true;
      cur_note_states[note] = RELEASED;
    }
    else
      cur_note_states[note] = STILLRELEASED;
  }

  // Preprocessing 1. If octave is changed while note is held, pretend the held note is newly pressed.
  if (get_prev_actual_octave() != get_cur_actual_octave() && is_any_note_playing()) {
    pressed_note = held_note;
    held_note = NIL;
  }

  // Case 1. If a note is newly pressed, it is automatically the new current note. Kill all other notes.
  if (pressed_note != NIL) {
    kill_notes();
    cur_note_states[pressed_note] = PRESSED;
  }
  // Case 2.
  // If no new pressed notes but a note was released, then play out the held_note.
  // We make its state HELD if it is the currently playing note (so as to prevent a new attack),
  // and PRESSED otherwise.
  // e.g. Suppose notes C and G are pressed in that order and held, so G is currently playing.
  // If C is released, then G (the most recent held note) will be set to HELD.
  // If G is released instead, then C will be set to PRESSED.
  else if (pressed_note == NIL && any_released && held_note != NIL) {
    int cur_note = get_cur_note();
    kill_notes();
    cur_note_states[held_note] = (held_note == cur_note) ? HELD : PRESSED;
  }
  // Case 3. If no new pressed or released notes but notes are still held, then stick with the
  // currently playing note.
  else if (pressed_note == NIL && !any_released && held_note != NIL) { 
    int cur_note = get_cur_note();
    kill_notes();
    if (cur_note != NIL)
      cur_note_states[cur_note] = HELD;
  }
}


/** *****************************/
/** CHORD MODE                  */
/** *****************************/

static void update_note_state_in_chord_mode() {
  update_notetables();
  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    if (get_cur_note_states()[note] == RELEASED)
      cur_note_states[note] = STILLRELEASED;
    else if (notetable_prev[note] && notetable[note])
      cur_note_states[note] = HELD;
    else if (!notetable_prev[note] && notetable[note])
      cur_note_states[note] = PRESSED;
    else if (notetable_prev[note] && !notetable[note])
      cur_note_states[note] = RELEASED;
  }
}

void update_note_state() {
  if (is_chord_mode()) update_note_state_in_chord_mode();
  else update_note_state_in_solo_mode();
}
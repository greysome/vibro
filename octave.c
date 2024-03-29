#include "octave.h"

static int global_octave = 0;
static int local_octave_modifier = 0;
static int prev_global_octave;
static int prev_local_octave_modifier;

// note.h cannot be included so I'll just redefine the constant here.
#define NOTETABLE_SIZE 33
// We store the octaves of the notes when they are released, because the octave
// of a note in the RELEASE state of the ADSR envelope shouldn't change.
static int octaves_on_release[NOTETABLE_SIZE];

void update_global_octave() {
  prev_global_octave = global_octave;
  if (IsKeyPressed(KEY_DOWN))
    global_octave = clamp(global_octave - 1, MIN_OCTAVE, MAX_OCTAVE);
  if (IsKeyPressed(KEY_UP))
    global_octave = clamp(global_octave + 1, MIN_OCTAVE, MAX_OCTAVE);
}

int get_global_octave() {
  return global_octave;
}

void update_local_octave_modifier() {
  prev_local_octave_modifier = local_octave_modifier;
  int global_octave = get_global_octave();
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && global_octave > MIN_OCTAVE)
    local_octave_modifier = -1;
  else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && global_octave < MAX_OCTAVE)
    local_octave_modifier = 1;
  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    local_octave_modifier = 0;
}

int get_local_octave_modifier() {
  return local_octave_modifier;
}

int get_note_octave_modifier(int note) {
  if (note == 0)
    return -1;
  else if (note >= 13)
    return 1;
  else if (note >= 26)
    return 2;
  return 0;
}

int get_cur_actual_octave() {
  return global_octave + local_octave_modifier;
}

int get_prev_actual_octave() {
  return prev_global_octave + prev_local_octave_modifier;
}

int *get_octaves_on_release() {
  return octaves_on_release;
}

void update_octave_on_release(int note) {
  assert(note >= 0 && note < NOTETABLE_SIZE);
  octaves_on_release[note] = get_cur_actual_octave();
}
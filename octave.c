/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "octave.h"
#include "keys.h"

static int global_octave = 0;
static int local_octave_modifier = 0;
static int prev_global_octave;
static int prev_local_octave_modifier;

void update_global_octave() {
  prev_global_octave = global_octave;
  if (IsKeyPressed(KEY_DOWN))
    global_octave = clamp(global_octave - 1, MINOCTAVE, MAXOCTAVE);
  if (IsKeyPressed(KEY_UP))
    global_octave = clamp(global_octave + 1, MINOCTAVE, MAXOCTAVE);
}

int get_global_octave() {
  return global_octave;
}

void update_local_octave_modifier() {
  prev_local_octave_modifier = local_octave_modifier;
  int global_octave = get_global_octave();
  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && global_octave > MINOCTAVE)
    local_octave_modifier = -1;
  else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && global_octave < MAXOCTAVE)
    local_octave_modifier = 1;
  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    local_octave_modifier = 0;
}

int get_local_octave_modifier() {
  return local_octave_modifier;
}

int get_note_octave_modifier() {
  // TODO: replace the 12 with a named constant
  int note = get_cur_note();
  if (note == -1)
    return -1;
  else if (note >= 12)
    return 1;
  else if (note >= 25)
    return 2;
  return 0;
}

int get_cur_actual_octave() {
  return global_octave + local_octave_modifier;
}

int get_prev_actual_octave() {
  return prev_global_octave + prev_local_octave_modifier;
}

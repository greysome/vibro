#include "freq.h"

#define MAX_DIVE_FRAMES (FPS / 2)
#define DIVE_FREQ_STEP 0.95
#define GLISS_FREQ_STEP 1.0194

// Pitch bends are controlled via mousewheel scrolling.
static float bend_modifier = 1;
// Glisses are controlled via up-and-down mouse movement.
static float gliss_modifier = 1;
// Vibrato is controlled by SPACE key.
static float vib_modifier = 1;
// Dive effect is activated by ALT key.
static float dive_modifier = 1;

// Autogliss is activated when the user glisses while pressing a new note, and
// without releasing the previous note. The speed of the vertical mouse movement
// (measured in prev_mouse_dy) determines the number of frames to autogliss
// (stored in autogliss_total_frames). At each frame, the frequency is scaled by
// a factor of autogliss_freq_step, starting from autogliss_start_freq (i.e. the
// actual frequency right before new note was pressed); autogliss_frame_counter
// is also incremented.

static int prev_mouse_dy = 0;
static int autogliss_total_frames = 0;
static int autogliss_frame_counter = 0;
static float autogliss_freq_step = 1;
static float autogliss_start_freq;

float get_note_freq(int note, int octave) {
  return C4 * pow(2, octave) * pow(SEMITONE, note);
}

float get_actual_freq(int note, int octave) {
  return get_note_freq(note, octave) * bend_modifier * gliss_modifier * vib_modifier * dive_modifier;
}

void update_pitch_bend() {
  // We keep track of the number of frames the user hasn't scrolled, so that
  // when it reaches a certain amount we snap the pitch bend modifier to the
  // nearest semitone.
  static int frames_not_scrolled = 0;

  // Reset pitch bend when there is a new note.
  if (is_any_note_pressed()) {
    frames_not_scrolled = 0;
    bend_modifier = 1;
  }

  float dy = GetMouseWheelMove();
  if (dy == 0)
    frames_not_scrolled++;
  else
    bend_modifier *= powf(GLISS_FREQ_STEP, dy);

  if (frames_not_scrolled >= 5) {
    frames_not_scrolled = 0;
    // Snap to nearest semitone
    float num_semitones = logf(bend_modifier) / logf(SEMITONE);
    bend_modifier = powf(SEMITONE, roundf(num_semitones));
  }
}

static bool is_autoglissing() {
  NoteState s = get_cur_note_state();
  return is_solo_mode() && autogliss_frame_counter < autogliss_total_frames &&
    (s == PRESSED || s == HELD);
}

void update_gliss() {
  // 1. Ignore gliss when autoglissing
  // 2. Reset gliss when a new note is pressed
  if (is_autoglissing() || is_any_note_pressed()) {
    gliss_modifier = 1;
    return;
  }

  prev_mouse_dy = mouse_dy;
  float factor = 1.0002;
  if (mouse_dy != 0)
    gliss_modifier /= pow(factor, mouse_dy);
  gliss_modifier = clamp(gliss_modifier, 0.5, 2);
}

void update_dive() {
  static int frames_dived = 0;
  // Activate dive when ALT key is pressed.
  if (IsKeyPressed(KEY_LEFT_ALT) || IsKeyPressed(KEY_RIGHT_ALT))
    frames_dived = 1;
  // Dive is active only when frames_dived > 0
  if (frames_dived == 0)
    return;
  if (++frames_dived <= MAX_DIVE_FRAMES)
    // Reset dive when new note is pressed.
    if (is_any_note_pressed()) {
      frames_dived = 0;
      dive_modifier = 1;
    }
    else
      dive_modifier *= DIVE_FREQ_STEP;
  else {
    kill_notes();
    frames_dived = 0;
    dive_modifier = 1;
  }
}

void update_vib() {
  // There are two parameters controlling vibrato: the frequency and length of
  // the SPACE presses. The former is measured by frames_space_up and
  // controls vibrato speed (vib_speed), while the latter is measured by
  // frames_space_down and controls vibrato depth (vib_depth).

  // The actual vibrato works by oscillating the frequency according to a sine
  // wave. The current phase of the wave is stored in vib_phase.

  static int frames_space_down = 0;
  static int frames_space_up = 0;
  static float vib_speed = 0;
  static float vib_depth = 1;
  static float vib_phase = 0;

  static bool space_down;
  static bool prev_space_down = false;

  // Momentarily kill vibrato when a new note is pressed.
  if (is_any_note_pressed()) {
    vib_depth = 1;
    vib_modifier = 1;
    frames_space_down = 0;
    frames_space_up = 0;
    return;
  }

  prev_space_down = space_down;
  space_down = IsKeyDown(KEY_SPACE);

  // SPACE was just pressed
  if (!prev_space_down && space_down) {
    vib_speed = 1.0 / frames_space_up;
    frames_space_up = 0;
  }

  // SPACE was just released
  if (prev_space_down && !space_down) {
    vib_depth = clamp(pow(1.003, frames_space_down), 1, 1.04);
    frames_space_down = 0;
  }

  // SPACE is down
  if (space_down)
    frames_space_down++;
  else {
    frames_space_up++;
    // Make vibrato decay if SPACE is not pressed continually
    if (frames_space_up > 30) {
      vib_speed = 0;
      vib_phase = 0;
      vib_modifier += 0.2 * (1.0 - vib_modifier);
      return;
    }
  }

  vib_phase += vib_speed / 2;
  if (vib_phase >= 1)
    vib_phase -= 1;
  vib_modifier = pow(vib_depth, sinf(vib_phase * 2 * PI));
}

void update_autogliss() {
  // Autogliss is only activated in solo mode when a new note is pressed while glissing on the previous note
  if (is_chord_mode()) return;
  if (is_legato() && prev_mouse_dy) {
    no_attack(); // We don't want to attack the new note!
    autogliss_start_freq = get_actual_freq(get_prev_note() - 1, get_prev_actual_octave());
    // Reset gliss; moving mouse vertically now does nothing until autogliss is complete
    gliss_modifier = 1;
    int cur_note_freq = get_note_freq(get_cur_note() - 1, get_cur_actual_octave());
    autogliss_total_frames = abs(cur_note_freq - autogliss_start_freq) / powf(-prev_mouse_dy, 0.5);
    autogliss_total_frames = clamp(autogliss_total_frames, 5, 40);
    autogliss_freq_step = powf(cur_note_freq / autogliss_start_freq, 1.0 / autogliss_total_frames);
    autogliss_frame_counter = 0;
  }

  if (is_autoglissing())
    autogliss_frame_counter++;
  else {
    autogliss_frame_counter = 0;
    autogliss_total_frames = 0;
    autogliss_freq_step = 1;
  }
}

float *get_cur_actual_freqs() {
  static float freqs[NOTETABLE_SIZE];
  if (is_solo_mode() && is_any_note_playing() && is_autoglissing()) {
    freqs[get_cur_note()] = autogliss_start_freq * powf(autogliss_freq_step, autogliss_frame_counter);
  }
  else {
    for (int i = 0; i < NOTETABLE_SIZE; i++)
      freqs[i] = get_actual_freq(i-1, get_cur_actual_octave());
  }
  return freqs;
}

void reset_freq_modifiers() {
  bend_modifier = 1;
  gliss_modifier = 1;
  vib_modifier = 1;
  dive_modifier = 1;
}
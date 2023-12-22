/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "freq.h"
#include "keys.h"

#define MAXDIVEFRAMES (FPS / 2)

// Fine-control of frequency via the mousewheel. For pitch bends.
static float bend_modifier = 1;
// Changing frequency via up-and-down mouse movement. For glisses.
static float gliss_modifier = 1;
// Changing frequency via space bar. For vibrato.
static float vib_modifier = 1;
// Changing frequency from the dive effect.
static float dive_modifier = 1;

static int prevmousedy = 0;
static int autogliss_frame_count = 0;
static int autogliss_cur_frames = 0;
static float autogliss_freq_step = 1;
static float autogliss_startfreq;

float get_note_freq(int note, int octave) {
  return C4 * pow(2, octave) * pow(SEMITONE, note);
}

float get_actual_freq(int note, int octave) {
  return get_note_freq(note, octave) * bend_modifier * gliss_modifier * vib_modifier * dive_modifier;
}

float get_cur_note_freq() {
  return get_note_freq(get_cur_note(), get_cur_actual_octave());
}

float get_prev_note_freq() {
  return get_note_freq(get_prev_note(), get_prev_actual_octave());
}

void update_pitch_bend() {
  static int is_scrolling = 0;
  // Number of frames that user hasn't scrolled. Used to determine when to snap
  // the pitch bend to the nearest semitone.
  static int frames_noscroll = 0;

  if (get_cur_note_state() == PRESSED) {
    frames_noscroll = 0;
    is_scrolling = 0;
    bend_modifier = 1;
  }
  float dy = GetMouseWheelMove();
  if (dy > 0) {
    bend_modifier *= pow(1.0194, dy);
    is_scrolling = 1;
  } else if (dy < 0) {
    bend_modifier /= pow(1.0194, -dy);
    is_scrolling = 1;
  } else
    frames_noscroll++;

  if (frames_noscroll >= 5) {
    frames_noscroll = 0;
    // Snap to nearest semitone
    if (is_scrolling) {
      // Sorry for messy code I'll fix this later I promise

      // Upwards
      if (sqrt(SEMITONE) < bend_modifier &&
          bend_modifier < SEMITONE * sqrt(SEMITONE))
        bend_modifier = SEMITONE;
      else if (SEMITONE * sqrt(SEMITONE) <= bend_modifier &&
               bend_modifier < SEMITONE * SEMITONE * sqrt(SEMITONE))
        bend_modifier = SEMITONE * SEMITONE;
      else if (SEMITONE * SEMITONE * sqrt(SEMITONE) <= bend_modifier &&
               bend_modifier <
                   SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE))
        bend_modifier = SEMITONE * SEMITONE * SEMITONE;
      else if (SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE) <=
                   bend_modifier &&
               bend_modifier <
                   SEMITONE * SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE))
        bend_modifier = SEMITONE * SEMITONE * SEMITONE * SEMITONE;

      // Downwards
      else if (1.0 / (SEMITONE * sqrt(SEMITONE)) <= bend_modifier &&
               bend_modifier < 1.0 / sqrt(SEMITONE))
        bend_modifier = 1.0 / SEMITONE;
      else if (1.0 / (SEMITONE * SEMITONE * sqrt(SEMITONE)) <=
                   bend_modifier &&
               bend_modifier < 1.0 / (SEMITONE * sqrt(SEMITONE)))
        bend_modifier = 1.0 / (SEMITONE * SEMITONE);
      else if (1.0 / (SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE)) <=
                   bend_modifier &&
               bend_modifier < 1.0 / (SEMITONE * SEMITONE * sqrt(SEMITONE)))
        bend_modifier = 1.0 / (SEMITONE * SEMITONE * SEMITONE);
      else if (1.0 / (SEMITONE * SEMITONE * SEMITONE * SEMITONE *
                      sqrt(SEMITONE)) <=
                   bend_modifier &&
               bend_modifier <
                   1.0 / (SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE)))
        bend_modifier = 1.0 / (SEMITONE * SEMITONE * SEMITONE * SEMITONE);

      else
        bend_modifier = 1.0;
    }
  }
}

static bool is_autoglissing() {
  return autogliss_cur_frames < autogliss_frame_count && get_cur_note_state() != RELEASED;
}

void update_gliss() {
  if (is_autoglissing()) {
    gliss_modifier = 1;
    return;
  }
  // Reset gliss if note released and not autoglissing
  if (get_cur_note_state() == RELEASED || get_cur_note_state() == PRESSED) {
    gliss_modifier = 1;
    return;
  }
  prevmousedy = mousedy;
  float factor = 1.0002;
  if (mousedy > 0)
    gliss_modifier /= pow(factor, mousedy);
  else if (mousedy < 0)
    gliss_modifier *= pow(factor, -mousedy);
  gliss_modifier = clamp(gliss_modifier, 0.5, 2);
}

void update_effects() {
  static int frames_dive = 0;
  if (IsKeyPressed(KEY_LEFT_ALT) || IsKeyPressed(KEY_RIGHT_ALT))
    frames_dive = 1;

  if (frames_dive > 0) {
    if (++frames_dive <= MAXDIVEFRAMES)
      if (get_cur_note_state() == PRESSED) {
        frames_dive = 0;
        dive_modifier = 1;
      } else
        dive_modifier *= 0.95;
    else {
      kill_note();
      frames_dive = 0;
      dive_modifier = 1;
    }
  }
}

void update_vib() {
  static NoteState prev_vib_state;
  static NoteState cur_vib_state = IDLE;
  // The two parameters that control vibrato: frequency and length of
  // pressing spacebar. The former controls speed and the latter controls
  // depth.
  static int frames_onspace, frames_betweenspace;
  static float vib_speed = 0;
  static float vib_depth = 1;
  static float vib_phase = 0;

  bool previously_doing_vib = (prev_vib_state == PRESSED) || (prev_vib_state == HELD);
  bool doing_vib = (cur_vib_state == PRESSED) || (cur_vib_state == HELD);

  // Momentarily kill vibrato when a new note is pressed
  if (get_cur_note_state() == PRESSED) {
    vib_depth = 1;
    cur_vib_state = IDLE;
    frames_onspace = 0;
    frames_betweenspace = 0;
    vib_modifier = 1;
    return;
  }

  vib_phase += vib_speed / FPS;
  prev_vib_state = cur_vib_state;

  if (IsKeyDown(KEY_SPACE))
    cur_vib_state = doing_vib ? HELD : PRESSED;
  else
    cur_vib_state = doing_vib ? RELEASED : IDLE;

  // Just pressed space
  if (!previously_doing_vib && doing_vib) {
    vib_speed = 1.0 / frames_betweenspace;
    frames_betweenspace = 0;
  } else {
    frames_betweenspace++;
    if (frames_betweenspace > 30) {
      // Make vibrato decay if not 'replenished'
      vib_speed = 0;
      vib_phase = 0;
      vib_modifier += 0.2 * (1.0 - vib_modifier);
      return;
    }
  }

  // Just released space
  if (previously_doing_vib && !doing_vib) {
    vib_depth = clamp(pow(1.003, frames_onspace), 1, 1.04);
    frames_onspace = 0;
  }
  // Still pressing on space
  if (doing_vib)
    frames_onspace++;

  vib_phase += vib_speed;
  if (vib_phase >= 1)
    vib_phase = 0;
  vib_modifier = pow(vib_depth, sinf(vib_phase * 2 * PI));
}

void update_autogliss() {
  // Autogliss is only activated when new note is pressed while glissing on the previous note
  if (is_legato() && prevmousedy) {
    no_attack();
    autogliss_cur_frames = 0;
    autogliss_startfreq = get_actual_freq(get_prev_note(), get_prev_actual_octave());
    // Reset gliss; moving mouse vertically now does nothing until autogliss is complete
    gliss_modifier = 1;
    autogliss_frame_count = abs(get_cur_note_freq() - autogliss_startfreq) / powf(-prevmousedy,0.5);
    //printf("%f %d %f %d\n", abs(get_cur_note_freq()-autogliss_startfreq), prevmousedy, powf(-prevmousedy, 0.5), autogliss_frame_count);
    autogliss_frame_count = clamp(autogliss_frame_count, 8, 40);
    autogliss_freq_step = powf(get_cur_note_freq() / autogliss_startfreq, 1.0 / autogliss_frame_count);
  }

  if (is_autoglissing()) {
    autogliss_cur_frames++;
  }
  else {
    autogliss_cur_frames = 0;
    autogliss_frame_count = 0;
    autogliss_freq_step = 1;
  }
}

float get_cur_actual_freq() {
  // If currently autoglissing
  if (is_autoglissing())
    return autogliss_startfreq * powf(autogliss_freq_step, autogliss_cur_frames);
  else
    return get_cur_note_freq() * bend_modifier * gliss_modifier * vib_modifier * dive_modifier;
}
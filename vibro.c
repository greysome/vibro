/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "freq.h"
#include "raylib.h"
#include "tinywav/tinywav.h"

#include "util.h"
#include "globals.h"
#include "synthesise.h"
#include "gui.h"

void draw() {
  ClearBackground(BG);
  draw_wave();
  display_octave_text();
  display_note_text();
  display_mode_text();
  draw_volume_level();
}

void process_keyboard_inputs() {
  // Local/global octave needs to be updated BEFORE note state.
  // This is because a change in octave will cause change in note state,
  // even if the same key is being held.
  update_global_octave();
  update_local_octave_modifier();
  update_note_state();

  update_pitch_bend();
  update_autogliss();
  update_gliss();
  update_effects();
  update_vib();

  update_note_vol();
  apply_adsr();
}

int main() {
  InitWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "vibro");
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);

  AudioStream stream = LoadAudioStream(SAMPLE_RATE, BIT_DEPTH, 1);
  SetAudioStreamCallback(stream, write_audio_samples);
  PlayAudioStream(stream);

  DisableCursor();
  SetTargetFPS(FPS);

  while (!WindowShouldClose()) {
    screen_width = GetScreenWidth();
    screen_height = GetScreenHeight();

    mouse_dx = GetMouseDelta().x;
    mouse_dy = GetMouseDelta().y;
    if (abs(mouse_dx) >= abs(mouse_dy)) mouse_dy = 0;
    else mouse_dx = 0;

    process_keyboard_inputs();

    if (IsKeyPressed(KEY_LEFT_CONTROL)) {
      toggle_chord_mode();
      reset_freq_modifiers();
    }

    if (IsKeyPressed(KEY_GRAVE)) {
      if (!is_recording)
        tinywav_open_write(&tw, 1, SAMPLE_RATE, TW_INT16, TW_INLINE, RECORD_FILE);
      else
        tinywav_close_write(&tw);
      is_recording = !is_recording;
    }

    if (IsKeyPressed(KEY_TAB)) {
      if (IsWindowFullscreen()) {
        ToggleFullscreen();
        SetWindowSize(INITIAL_WIDTH, INITIAL_HEIGHT);
      } else {
        SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
        ToggleFullscreen();
      }
    }

    BeginDrawing();
    draw();
    EndDrawing();
  }

  if (is_recording)
    tinywav_close_write(&tw);
  CloseWindow();
  UnloadAudioStream(stream);
  CloseAudioDevice();

  return 0;
}
/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "raylib.h"
#include "tinywav/tinywav.h"

#include "util.h"
#include "globals.h"
#include "synthesise.h"
#include "input.h"
#include "gui.h"

void draw() {
  ClearBackground(BG);

  draw_wave();

  display_octave_text();
  display_note_text();
  draw_volume_level();
}

int main() {
  InitWindow(STARTINGWIDTH, STARTINGHEIGHT, "vibro");
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAXSAMPLES_PER_UPDATE);

  AudioStream stream = LoadAudioStream(SAMPLERATE, BITDEPTH, 1);
  SetAudioStreamCallback(stream, write_audio_samples);
  PlayAudioStream(stream);

  DisableCursor();
  SetTargetFPS(FPS);
  font = GetFontDefault();

  while (!WindowShouldClose()) {
    screenwidth = GetScreenWidth();
    screenheight = GetScreenHeight();

    mousedx = GetMouseDelta().x;
    mousedy = GetMouseDelta().y;
    if (abs(mousedx) >= abs(mousedy)) mousedy = 0;
    else mousedx = 0;

    process_keyboard_inputs();

    if (IsKeyPressed(KEY_GRAVE)) {
      if (!isrecording)
        tinywav_open_write(&tw,
                           1,  // number of channels
                           SAMPLERATE, TW_INT16, TW_INLINE, RECORDFILE);
      else
        tinywav_close_write(&tw);
      isrecording = !isrecording;
    }

    if (IsKeyPressed(KEY_TAB)) {
      if (IsWindowFullscreen()) {
        ToggleFullscreen();
        SetWindowSize(STARTINGWIDTH, STARTINGHEIGHT);
      } else {
        SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
        ToggleFullscreen();
      }
    }

    BeginDrawing();
    draw();
    EndDrawing();
  }

  if (isrecording)
    tinywav_close_write(&tw);
  CloseWindow();
  UnloadAudioStream(stream);
  CloseAudioDevice();

  return 0;
}
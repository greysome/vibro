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
#include "play_mode.h"
#include "instrument_mode.h"

typedef enum {
  PLAY_MODE, INSTRUMENT_MODE
} GuiMode;

int main() {
  InitWindow(INITIAL_WIDTH, INITIAL_HEIGHT, "vibro");
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAX_SAMPLES_PER_UPDATE);

  AudioStream stream = LoadAudioStream(SAMPLE_RATE, BIT_DEPTH, 1);
  SetAudioStreamCallback(stream, write_audio_samples);
  PlayAudioStream(stream);

  DisableCursor();
  SetTargetFPS(FPS);

  GuiMode gui_mode = PLAY_MODE;

  while (!WindowShouldClose()) {
    screen_width = GetScreenWidth();
    screen_height = GetScreenHeight();

    if (IsKeyPressed(KEY_TAB)) {
      if (IsWindowFullscreen()) {
        ToggleFullscreen();
        SetWindowSize(INITIAL_WIDTH, INITIAL_HEIGHT);
      } else {
        SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
        ToggleFullscreen();
      }
    }

    if (IsKeyPressed(KEY_LEFT)) {
      if (gui_mode == PLAY_MODE)
	gui_mode = INSTRUMENT_MODE;
      else
	gui_mode--;
    }
    else if (IsKeyPressed(KEY_RIGHT)) {
      if (gui_mode == INSTRUMENT_MODE)
	gui_mode = PLAY_MODE;
      else
	gui_mode++;
    }

    switch (gui_mode) {
    case PLAY_MODE: play_mode_gui(); break;
    case INSTRUMENT_MODE: instrument_mode_gui(); break;
    }
  }

  if (is_recording)
    tinywav_close_write(&tw);
  CloseWindow();
  UnloadAudioStream(stream);
  CloseAudioDevice();

  return 0;
}
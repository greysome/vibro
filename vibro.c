#include <math.h>
#include <stdio.h>
#include <string.h>
#include "tinywav/tinywav.h"
#include "raylib.h"
#include "util.h"
#include "globals.h"
#include "freq.h"
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
  add_instrument();

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


#define SHIFT_LEFT (SHIFT_DOWN && IsKeyPressed(KEY_LEFT))
#define SHIFT_RIGHT (SHIFT_DOWN && IsKeyPressed(KEY_RIGHT))

    // Do stuff before switching modes
    if (SHIFT_LEFT || SHIFT_RIGHT) {
      if (gui_mode == INSTRUMENT_MODE)
	commit_instrument_mode_changes();
    }

    // Decide mode to switch to
    if (SHIFT_LEFT) {
      if (gui_mode == PLAY_MODE)
	gui_mode = INSTRUMENT_MODE;
      else
	gui_mode--;
    }
    else if (SHIFT_RIGHT) {
      if (gui_mode == INSTRUMENT_MODE)
	gui_mode = PLAY_MODE;
      else
	gui_mode++;
    }

    // Do stuff after switching modes
    if (SHIFT_LEFT || SHIFT_RIGHT) {
      if (gui_mode == INSTRUMENT_MODE) {
	kill_notes();
	kill_vols();
	reset_entryrow();
	load_instrument_mode_state(get_cur_instrument_idx());
      }
    }

    if (gui_mode == PLAY_MODE) {
      if (IsKeyPressed(KEY_LEFT))
	select_previous_instrument();
      if (IsKeyPressed(KEY_RIGHT))
	select_next_instrument();
    }

    switch (gui_mode) {
    case PLAY_MODE: play_mode_gui(); break;
    case INSTRUMENT_MODE: instrument_mode_gui(); break;
    }
  }

  cleanup_instruments();
  if (gui_mode == INSTRUMENT_MODE)
    cleanup_instrument_mode_state();

  if (is_recording)
    tinywav_close_write(&tw);
  CloseWindow();
  UnloadAudioStream(stream);
  CloseAudioDevice();

  return 0;
}
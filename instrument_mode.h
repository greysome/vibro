#ifndef _INSTRUMENT_MODE
#define _INSTRUMENT_MODE

#include <string.h>
#include "util.h"
#include "raylib.h"
#include "globals.h"
#include "gui.h"

#define MAX_STR_LEN 100

typedef enum {
  PULSE, TRI, SAW, SAMPLE
} WaveType;

typedef struct {
  char name[MAX_STR_LEN];
  WaveType wave_type;

  float pulse_width;
  bool tri_nes_style;
  bool saw_nes_style;

  char sample_path[MAX_STR_LEN];
  bool sample_changed;
  bool sample_ready;
  float sample_pitch_modifier;
  bool sample_play_continuously;
  bool sample_stop_on_release;
  Sound sample;
  int sample_rate;
  int sample_data_length;
  float *sample_data;
} Instrument;

void instrument_mode_gui();
void init_instrument();
void reload_instrument_sample_if_changed();
Instrument get_instrument();

#endif
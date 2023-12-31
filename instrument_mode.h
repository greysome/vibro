#ifndef _INSTRUMENT_MODE
#define _INSTRUMENT_MODE

#include "util.h"
#include "raylib.h"
#include "globals.h"
#include "gui.h"

typedef enum {
  PULSE, TRI, SAW
} WaveType;

typedef struct {
  WaveType wave_type;
  float pulse_width;
  bool tri_nes_style;
  bool saw_nes_style;
} Instrument;

void instrument_mode_gui();
Instrument get_instrument();

#endif
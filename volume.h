#ifndef VOLUME
#define VOLUME

#include "raylib.h"
#include "globals.h"
#include "note.h"
#include "util.h"
#include "instrument_mode.h"

typedef enum {
  ATTACK, DECAY, SUSTAIN, RELEASE
} ADSRState;

void update_note_vol();
void apply_adsr();
float get_note_vol();
float *get_actual_vols();
void kill_vols();
bool is_silent();

#endif
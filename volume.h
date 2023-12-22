/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef VOLUME
#define VOLUME

#include "raylib.h"
#include "globals.h"
#include "keys.h"

typedef enum {
  ATTACK, DECAY, SUSTAIN, RELEASE
} ADSRState;

void update_sustain_vol();
void apply_adsr();
float get_actual_vol();
float get_sustain_vol();

#endif
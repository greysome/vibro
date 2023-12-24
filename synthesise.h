/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef SYNTHESISE
#define SYNTHESISE

#include "tinywav/tinywav.h"
#include "globals.h"
#include "util.h"
#include "freq.h"
#include "volume.h"

/** Synthesis functions. Outputs a value between -1 and 1. **/
float tri(float phase);
float nes_tri(float phase);
float nes_saw(float phase);
float nes_pulse(float phase);
float add_synthesise(float phase);
float get_amplitude(float *phases);
void write_audio_samples(void *buffer, unsigned int frames);

#endif
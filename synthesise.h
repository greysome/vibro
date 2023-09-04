/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef SYNTHESISE
#define SYNTHESISE

#include "globals.h"
#include "tinywav/tinywav.h"
#include "util.h"

/** Synthesis functions. Outputs a value between -1 and 1. **/
float tri(float phase) {
  return 4 * abs(phase - 0.5) - 1;
}

float nes_tri(float phase) {
  float y = tri(phase);
  return y - fmodf2(y, 2.0 / 16);
}

float nes_saw(float phase) {
  float y = 2 * (phase - 0.5);
  return y - fmodf2(y, 2.0 / 16);
}

float nes_pulse(float phase) {
  return phase >= curpulsewidth ? -1 : 1;
}

#define C addsynth_coeffs
float add_synthesise(float phase) {
  float output = 0;
  float amplitude = 0;
  for (int i = 0; i < NUM_HARMONICS; i++) {
    amplitude += C[i];
    output += C[i] * sinf(phase * 2 * (i + 1) * PI);
  }
  return output / amplitude;

  //float factor =
  //    C[0] + C[1] + C[2] + C[3] + C[4] + C[5] + C[6] + C[7] + C[8] + C[9];
  //return (C[0] * sinf(phase * 2 * PI) + C[1] * sinf(phase * 4 * PI) +
  //        C[2] * sinf(phase * 6 * PI) + C[3] * sinf(phase * 8 * PI) +
  //        C[4] * sinf(phase * 10 * PI) + C[5] * sinf(phase * 12 * PI) +
  //        C[6] * sinf(phase * 14 * PI) + C[7] * sinf(phase * 16 * PI) +
  //        C[8] * sinf(phase * 18 * PI) + C[9] * sinf(phase * 20 * PI)) /
  //       factor;
}
#undef C

void synthesise(void* buffer, unsigned int frames) {
  short* d = (short*)buffer;
  float wavbuf[frames];
  float amplitude;

  for (unsigned int i = 0; i < frames; i++) {
    // The actual synthesis
    if (wavetype == TRI)
      amplitude = nes_tri(curphase);
    else if (wavetype == SAW)
      amplitude = nes_saw(curphase);
    else if (wavetype == PULSE)
      amplitude = nes_pulse(curphase);
    else if (wavetype == SINE)
      amplitude = add_synthesise(curphase);
    // Scale it to match the actual amplitude for the output format
    amplitude *= actualvol * MAXVOL;
    d[i] = (short)(amplitude * pow(2, BITDEPTH));
    if (isrecording) {
      // For some reason, these changes need to be made to the
      // amplitude or else the .wav file will corrupt, particularly at
      // low volumes.
      // I have ZERO idea why that is the case.
      amplitude -= MAXVOL;
      amplitude -= fmodf2(amplitude, 0.01);
      wavbuf[i] = amplitude;
    }

    curphase += actualfreq / SAMPLERATE;
    if (curphase >= 1)
      curphase = fmodf(curphase, 1.0);
  }

  if (isrecording)
    tinywav_write_f(&tw, wavbuf, frames);
}

#endif
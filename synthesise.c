/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "synthesise.h"

static float cur_pulse_width = 0.5;
static float cur_phase = 0.0;

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
  return phase >= cur_pulse_width ? -1 : 1;
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
}
#undef C

void write_audio_samples(void *buffer, unsigned int frames) {
  short* d = (short*)buffer;
  float wavbuf[frames];
  float amplitude;

  for (unsigned int i = 0; i < frames; i++) {
    // The actual synthesis
    if (wavetype == TRI)
      amplitude = nes_tri(cur_phase);
    else if (wavetype == SAW)
      amplitude = nes_saw(cur_phase);
    else if (wavetype == PULSE)
      amplitude = nes_pulse(cur_phase);
    else if (wavetype == SINE)
      amplitude = add_synthesise(cur_phase);
    // Scale it to match the actual amplitude for the output format
    amplitude *= get_actual_vol() * MAXVOL;
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

    cur_phase += get_cur_actual_freq() / SAMPLERATE;
    if (cur_phase >= 1)
      cur_phase = fmodf(cur_phase, 1.0);
  }

  if (isrecording)
    tinywav_write_f(&tw, wavbuf, frames);
}


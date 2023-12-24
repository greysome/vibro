/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "synthesise.h"

static float cur_pulse_width = 0.5;
static float cur_phases[NOTETABLE_SIZE] = {0};

#define NUM_HARMONICS 20
static float addsynth_coeffs[NUM_HARMONICS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

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

float add_synthesise(float phase) {
  float output = 0;
  float amplitude = 0;
  for (int i = 0; i < NUM_HARMONICS; i++) {
    amplitude += addsynth_coeffs[i];
    output += addsynth_coeffs[i] * sinf(phase * 2 * (i + 1) * PI);
  }
  return output / amplitude;
}

float get_amplitude(float *phases) {
  float amplitude = 0;
  float vol = 0;
  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    vol = get_actual_vols()[note] * MAX_VOL;
    amplitude += vol * nes_pulse(phases[note]);
  }
  return amplitude;
}

void write_audio_samples(void *buffer, unsigned int frames) {
  short* d = (short*)buffer;
  float wavbuf[frames];
  float amplitude;
  static float prev;

  for (unsigned int i = 0; i < frames; i++) {
    prev = amplitude;
    amplitude = get_amplitude(cur_phases);
    amplitude = clamp(amplitude, -0.5, 0.5);
    d[i] = (short)(amplitude * pow(2, BIT_DEPTH));
    if (is_recording) {
      // For some reason, these changes need to be made to the
      // amplitude or else the .wav file will corrupt, particularly at
      // low volumes.
      // I have ZERO idea why that is the case.
      amplitude -= MAX_VOL;
      amplitude -= fmodf2(amplitude, 0.01);
      wavbuf[i] = amplitude;
    }

    for (int note = 0; note < NOTETABLE_SIZE; note++) {
      if (get_actual_vols()[note] == 0) continue;
      cur_phases[note] += get_cur_actual_freqs()[note] / SAMPLE_RATE;
      if (cur_phases[note] >= 1)
	cur_phases[note] -= 1.0;
    }
  }

  if (is_recording)
    tinywav_write_f(&tw, wavbuf, frames);
}

/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "synthesise.h"
#include "freq.h"
#include "volume.h"

static float cur_pulse_width = 0.5;
static float cur_phases[KEYTABLE_SIZE] = {0};

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

float get_amplitude(float *phases) {
  float amplitude = 0;
  float vol = 0;
  for (int note = 0; note < KEYTABLE_SIZE; note++) {
    vol = get_actual_vols()[note] * MAXVOL;
    // The actual synthesis
    if (wavetype == TRI) amplitude += vol * nes_tri(phases[note]);
    else if (wavetype == SAW) amplitude += vol * nes_saw(phases[note]);
    else if (wavetype == PULSE) amplitude += vol * nes_pulse(phases[note]);
    else if (wavetype == SINE) amplitude += vol * add_synthesise(phases[note]);
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

    for (int note = 0; note < KEYTABLE_SIZE; note++) {
      if (get_actual_vols()[note] == 0) continue;
      cur_phases[note] += get_cur_actual_freqs()[note] / SAMPLERATE;
      if (cur_phases[note] >= 1)
	cur_phases[note] -= 1.0;
    }
  }

  if (isrecording)
    tinywav_write_f(&tw, wavbuf, frames);
}

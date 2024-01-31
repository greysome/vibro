#include "synthesise.h"
// Defines sin_table
#include "sin_table.txt"

static float cur_phases[NOTETABLE_SIZE] = {0};

float pulse(float phase, float pulse_width) {
  return phase >= pulse_width ? -1 : 1;
}

float tri(float phase, bool nes_style) {
  float y = 4 * fabs(phase - 0.5) - 1;
  if (nes_style)
    y -= fmodf2(y, 2.0 / 16);
  return y;
}

float saw(float phase, bool nes_style) {
  float y = 2 * (phase - 0.5);
  if (nes_style)
    y -= fmodf2(y, 2.0 / 16);
  return y;
}

float my_sinf(float x) {
  float phase = fmodf2(x, 2*PI) / (2*PI);
  return sin_table[(int)(phase*1024)];
}

float sine(float phase, float *sine_coeffs) {
  float output = 0;
  float amplitude = 0;
  for (int i = 0; i < NUM_HARMONICS; i++) {
    amplitude += sine_coeffs[i];
    output += sine_coeffs[i] * my_sinf(phase*2*(i+1)*PI);
  }
  return output / amplitude;
}

float get_amplitude(float *phases) {
  if (get_num_instruments() == 0)
    return 0;
  Instrument instrument = *get_cur_instrument();
  float amplitude = 0;
  float vol = 0;
  float y;
  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    vol = get_actual_vols()[note] * MAX_VOL;
    switch (instrument.type) {
    case PULSE: y = pulse(phases[note], instrument.pulse_width); break;
    case TRI: y = tri(phases[note], instrument.tri_nes_style); break;
    case SAW: y = saw(phases[note], instrument.saw_nes_style); break;
    case SINE: y = sine(phases[note], instrument.sine_coeffs); break;
    case SAMPLE: case MULTISAMPLE: y = 0; break;
    }
    amplitude += vol * y;
  }
  return amplitude;
}

void write_audio_samples(void *buffer, unsigned int frames) {
  short* d = (short*)buffer;
  float wavbuf[frames];
  float amplitude;

  for (unsigned int i = 0; i < frames; i++) {
    amplitude = get_amplitude(cur_phases);
    amplitude = fclamp(amplitude, -0.5, 0.5);
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

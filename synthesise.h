#ifndef SYNTHESISE
#define SYNTHESISE

#include "tinywav/tinywav.h"
#include "globals.h"
#include "util.h"

/** Synthesis functions. Outputs a value between -1 and 1. **/
float tri(float phase) {
  return 4*abs(phase-0.5) - 1;
}

float nes_tri(float phase) {  
  float y = tri(phase);
  return y - fmodf2(y, 2.0/16);
}

float nes_saw(float phase) {
  float y = 2 * (phase-0.5);
  return y - fmodf2(y, 2.0/16);
}

float nes_pulse(float phase) {
  return phase >= curpulsewidth ? -1 : 1;
}

void synthesise(void *buffer, unsigned int frames) {
  short *d = (short *) buffer;
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
    // Scale it to match the actual amplitude for the output format
    amplitude *= actualvol * MAXVOL;
    d[i] = (short) (amplitude * pow(2, BITDEPTH));
    if (isrecording) {
      // For some reason, these changes need to be made to the
      // amplitude or else the .wav file will corrupt, particularly at
      // low volumes.
      // I have ZERO idea why that is the case.
      amplitude -= MAXVOL;
      amplitude -= fmodf2(amplitude, 0.01);
      wavbuf[i] = amplitude;
    }

    curphase += ACTUALFREQ / SAMPLERATE;
    if (curphase >= 1)
      curphase = fmodf(curphase, 1.0);
  }

  if (isrecording)
    tinywav_write_f(&tw, wavbuf, frames);
}

#endif
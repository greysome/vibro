#ifndef SYNTHESISE
#define SYNTHESISE

#include "tinywav/tinywav.h"
#include "globals.h"
#include "util.h"
#include "freq.h"
#include "volume.h"
#include "instrument.h"

// How loud should this program be compared to the actual
// system volume, from 0 to 1?
#define MAX_VOL 0.3

float get_amplitude(float *phases);
void write_audio_samples(void *buffer, unsigned int frames);

#endif
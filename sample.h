#ifndef _SAMPLE
#define _SAMPLE

#include "raylib.h"
#include "note.h"
#include "freq.h"
#include "volume.h"
#include "instrument_mode.h"

/** For displaying the sample waveform while playing back a sample **/
typedef struct {
  int sample_frame_counter;
  float pitch_modifier;
} SamplePlaybackState;

void play_sample();
SamplePlaybackState get_sample_playback_state();

#endif

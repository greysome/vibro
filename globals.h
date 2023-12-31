#ifndef GLOBALS
#define GLOBALS

#include "tinywav/tinywav.h"

#define FPS 60

// Parameters passed to the Raylib audio functions
// SetAudioStreamBufferSizeDefault() and LoadAudioStream()
#define MAX_SAMPLES_PER_UPDATE 9192
#define SAMPLE_RATE 96000
#define BIT_DEPTH 16
#define RECORD_FILE "out.wav"

// Are we recording?
extern bool is_recording;
// Struct to facilitate .wav output
extern TinyWav tw;

// At each frame, if the mouse x displacement (as measured by GetMouseDelta()) >
// mouse y displacement, then mouse_dx = mouse x displacement and mouse_dy = 0.
// Similarly if mouse x displacement < mouse y displacement.
// I programmed it this way to prevent accidental glissing (caused by vertical
// mouse movement) while changing volume (caused by horizontal mouse movement).
extern float mouse_dx;
extern float mouse_dy;

#endif
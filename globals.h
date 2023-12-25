#ifndef GLOBALS
#define GLOBALS

#include "raylib.h"
#include "tinywav/tinywav.h"

#define FPS 60

// Window dimensions upon program startup, and also after exiting fullscreen mode.
// May not be equal to *current* window dimensions (which is instead stored in
// screen_width/screen_height), as the user can freely resize the window.
#define INITIAL_WIDTH 1200
#define INITIAL_HEIGHT 700

// Parameters passed to the Raylib audio functions
// SetAudioStreamBufferSizeDefault() and LoadAudioStream()
#define MAX_SAMPLES_PER_UPDATE 9192
#define SAMPLE_RATE 96000
#define BIT_DEPTH 16

#define RECORD_FILE "out.wav"

// See comment above INITIAL_WIDTH/INITIAL_HEIGHT
extern int screen_width;
extern int screen_height;

// At each frame, if the mouse x displacement (as measured by GetMouseDelta()) >
// mouse y displacement, then mouse_dx = mouse x displacement and mouse_dy = 0.
// Similarly if mouse x displacement < mouse y displacement.
// I programmed it this way to prevent accidental glissing (caused by vertical
// mouse movement) while changing volume (caused by horizontal mouse movement).
extern float mouse_dx;
extern float mouse_dy;

// Are we recording?
extern bool is_recording;
// Struct to facilitate .wav output
extern TinyWav tw;

#endif
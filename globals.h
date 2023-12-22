/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef GLOBALS
#define GLOBALS

#include "raylib.h"
#include "tinywav/tinywav.h"
#include "util.h"

#define FPS 60
#define STARTINGWIDTH 1200
#define STARTINGHEIGHT 700

// These will be different from STARTINGWIDTH/HEIGHT if user goes to fullscreen mode.
extern int screenwidth;
extern int screenheight;

// For setting up Raylib audio
#define MAXSAMPLES_PER_UPDATE 9192
#define SAMPLERATE 96000
#define BITDEPTH 16

#define RECORDFILE "out.wav"
#define MINOCTAVE -4
#define MAXOCTAVE 2
// How loud should this program be compared to the actual
// system volume, from 0 to 1?
#define MAXVOL 0.3
// Number of seconds for the drawn wave to move one full cycle
#define WAVESPEED 2.0
// Controls the scale of the horizontal axis for the drawn wave.
// The lower, the more bunched up the wave will be
#define WAVEXSCALE 20000
// Scale of vertical axis.
#define WAVEYSCALE 100

#define TRI 0
#define SAW 1
#define PULSE 2
#define SINE 3
extern int wavetype;

/** Additive synthesis **/
#define NUM_HARMONICS 20
extern float addsynth_coeffs[NUM_HARMONICS];

/** Recording-related **/
// Are we recording?
extern int isrecording;
// Struct to facilitate .wav output
extern TinyWav tw;

/** Mouse input **/
// At each frame, exactly one of the variables will be set
// depending on whether the vertical or horizontal motion
// of the mouse is larger.
extern float mousedx;
extern float mousedy;

extern Font font;

#endif
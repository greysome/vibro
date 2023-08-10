/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef GLOBALS
#define GLOBALS

#include "raylib.h"
#include "tinywav/tinywav.h"
#include "util.h"

/** Global settings **/
#define NORMALWIDTH 1400
#define NORMALHEIGHT 900
#define WIDTH GetScreenWidth()
#define HEIGHT GetScreenHeight()
#define FPS 60
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

// Moving up a semitone increases the Hz by this factor
#define SEMITONE 1.05946
// In Hz
#define C4 261.6
#define CURFREQ                                                              \
  (C4 * pow(2, octave) * pow(SEMITONE, curnote) * freqoffset * freqoffset2 * \
   freqoffset3 * freqoffset_drop)

#define TRI 0
#define SAW 1
#define PULSE 2
int wavetype = PULSE;
// For pulse wave
float curpulsewidth;

/** Note/frequency **/
// The number of semitones above the C at the current octave.
// Only changes when a new note is pressed.
int curnote;
// Fine-control of frequency via the mousewheel. For pitch bends.
// Note freqoffset* is _multiplied_ in to get the final frequency.
float freqoffset = 1;
// Changing frequency via mouse movement. For glisses.
float freqoffset2 = 1;
int glisslock = 0;
// Changing frequency via space bar. For vibrato.
float freqoffset3 = 1;

// Pitch bend variables to do pitch correction
int frames_noscroll = 0;
int scrolled = 0;

// There are two ways to change the octave, via the up and down
// arrow keys and via left and right mouse clicks.
// The former method changes preoctave, whereas the latter
// changes octaveoffset. Thus the final octave is computed as
// preoctave + octaveoffset.
// C4 is considered to be octave 0.
int preoctave = 0;
int octaveoffset = 0;
int octave;

// How far are we in the current cycle of the wave from 0 to 1?
float curphase = 0.0;

/** Volume **/
// Volume of note when held, ignoring attacks and releases,
// as a proportion of MAXVOL.
// Can be controlled by the number keys on the keyboard.
float notevol = 1.0;
// Similar by notevol, but updates frame-by-frame according
// to attacks and releases
float curvol = 1.0;
// A setting that when enabled, always sets the volume to a fixed
// level when a new note is pressed. (This fixed level can only be
// changed with the volume control buttons 1 - 0.)
int constvol = 0;
// The aforementioned fixed level
// In contrast, notevol can be changed via dynamic volume control
// (i.e. moving the mouse left and right).
float startnotevol = 1.0;

/** Volume envelopes **/
// A volume envelope is hardcoded to have exactly 60 frames,
// which is also the set FPS. Each entry represents a proportion
// of notevol.
#define envel6(a, b, c, d, e, f)                                               \
  {                                                                            \
    a, a, a, a, a, a, a, a, a, a, b, b, b, b, b, b, b, b, b, b, c, c, c, c, c, \
        c, c, c, c, c, d, d, d, d, d, d, d, d, d, d, e, e, e, e, e, e, e, e,   \
        e, e, f, f, f, f, f, f, f, f, f, f,                                    \
  }

#define envel6_rapid_in(a, b, c, d, e, f)                                      \
  {                                                                            \
    a, a, a, b, b, b, c, c, c, d, d, d, e, e, e, f, f, f, 1, 1, 1, 1, 1, 1, 1, \
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,   \
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,                                    \
  }

#define envel6_rapid_out(a, b, c, d, e, f)                                     \
  {                                                                            \
    a, a, a, b, b, b, c, c, c, d, d, d, e, e, e, f, f, f, 0, 0, 0, 0, 0, 0, 0, \
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   \
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                    \
  }

#define NOENVEL_IN envel6_rapid_in(1, 1, 1, 1, 1, 1)
#define NOENVEL_OUT envel6_rapid_out(0, 0, 0, 0, 0, 0)

// float attackenvel[FPS] = envel6_rapid_in(1.5,1.2,1.1,1,1,1);
float attackenvel[FPS] = envel6_rapid_in(1, 1, 1, 1, 1, 1);
// float releaseenvel[FPS] = envel6_rapid_out(1,0.5,0.2,0,0,0);
// float releaseenvel[FPS] = envel6_rapid_out(1,0.2,0,0,0,0);
float releaseenvel[FPS] = {
    1, 1, 0.2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
float pulsewidthenvel[3] = {0.5, 0.5, 0.5};

/** Sustain **/
int sustain = 0;  // Sustain current note?

/** Note press data */
#define RELEASED 0
#define STILLRELEASED 1
#define PRESSED 2
#define STILLPRESSED 3
int curstate = STILLRELEASED;
int prevstate;
#define PLAYING ((curstate == PRESSED) || (curstate == STILLPRESSED))
// Number of frames elapsed since the last note that was played
// (including silence).
int frames_newnote;
// Number of frames elasped since note released.
int frames_releasenote;

#define NOTETABLE_SIZE 17
// Keeps track of which notes are pressed
// Used to determine which note to play out, in the case
// that multiple keys were pressed.
short notetable[NOTETABLE_SIZE];
short notetable_prev[NOTETABLE_SIZE];

/** Vibrato **/
int prevvibstate;
int curvibstate = STILLRELEASED;
int frames_onspace, frames_betweenspace;
float vibspeed = 0, vibdepth = 1;
float vibphase = 0;
#define PREVVIBPLAYING \
  ((prevvibstate == PRESSED) || (prevvibstate == STILLPRESSED))
#define VIBPLAYING ((curvibstate == PRESSED) || (curvibstate == STILLPRESSED))

/** Effects **/
#define MAXDROPFRAMES (FPS / 2)
int frames_drop = 0;
float freqoffset_drop = 1;

/** Text displays **/
const char* notetxt;
const char* octavetxt;
const char* voltxt;

/** Recording-related **/
// Are we recording?
int recording = 0;
// Struct to facilitate .wav output
TinyWav tw;

/** Mouse input **/
// At each frame, exactly one of the variables will be set
// depending on whether the vertical or horizontal motion
// of the mouse is larger.
float mousedx = 0;
float mousedy = 0;

#define FONTSIZE 40
#define XMARGIN 40
#define YMARGIN 40
#define YSPACE 50
#define DEFAULTFONT GetFontDefault()

#endif
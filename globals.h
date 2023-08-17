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
#define FPS 60
#define STARTINGWIDTH 1200
#define STARTINGHEIGHT 700
// These will be different from STARTINGWIDTH/HEIGHT if user goes to fullscreen
// mode.
int screenwidth;
int screenheight;
bool cursorenabled = false;

// For display text
#define FONTSIZE 20
#define XMARGIN 20
#define YMARGIN 20

Texture2D texture_pulsewave, texture_triwave, texture_sawwave;

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

// Moving up a semitone increases the Hz by this factor
#define SEMITONE 1.05946
// In Hz
#define C4 261.6

#define TRI 0
#define SAW 1
#define PULSE 2
int wavetype = PULSE;
// For pulse wave
float curpulsewidth = 0.5;
// How far are we in the current cycle of the wave from 0 to 1?
float curphase = 0.0;

/** Note/frequency **/
// The number of semitones above the C at the current octave.
// Only changes when a new note is pressed.
int curnote;
// Fine-control of frequency via the mousewheel. For pitch bends.
float freq_bend_factor = 1;
// Changing frequency via up-and-down mouse movement. For glisses.
float freq_gliss_factor = 1;
// Changing frequency via space bar. For vibrato.
float freq_vib_factor = 1;
// Changing frequency from the dive effect.
float freq_dive_factor = 1;
float actualfreq;

/* Gliss variables */
// Gliss lock means that moving mouse up and down doesn't perform a glissando.
// Useful because it is hard to change volume (left-and-right movement) without
// moving mouse up-and-down too.
int isglisslock = 0;

/* Autogliss variables */
int prevmousedy = 0;
int frames_toautogliss = 0;
float autogliss_startfreq, autogliss_endfreq;
float autogliss_freqstep;

/* Pitch bend variables to do pitch correction. */
// Number of frames that user hasn't scrolled. Used to determine when to snap
// the pitch bend to the nearest semitone.
int frames_noscroll = 0;
int isscrolling = 0;

// There are two ways to change the octave, via the up and down
// arrow keys and via left and right mouse clicks.
// The former method sets `globaloctave`, whereas the latter
// changes `octaveoffset`. Thus the actual octave being played
// is computed as `globaloctave + octaveoffset`.
// C4 is considered to be octave 0.
int globaloctave = 0;
int octaveoffset = 0;
int actualoctave = 0;
int prevactualoctave;

/** Volume **/
// Sustain volume as a proportion of MAXVOL.
// This can be controlled by the number keys on the keyboard,
// or by left-and-right mouse movement.
float sustainvol = 1.0;
// Updates frame-by-frame according to ADSR envelope
float actualvol = 0.0;
// A setting that when enabled, always sets sustainvol to a fixed
// level when a new note is pressed. (This fixed level can only be
// changed with the volume control buttons 1 - 0.)
int isconstvol = 0;
// The aforementioned fixed level
// In contrast, sustainvol can be changed via dynamic volume control
// (i.e. moving the mouse left and right).
float startnotevol = 1.0;

/** ADSR settings **/
#define ATTACK 0
#define DECAY 1
#define SUSTAIN 2
#define RELEASE 3
int ADSRstate = RELEASE;
int frames_into_sustain;
int attackframes = 1;
float attackpeak = 1;  // A proportion of sustainvol
float attackpeakvol;
int decayframes = 2;
int sustaindecayframes = 2000;
float releasepeak;  // Value of actualvol right when release starts
int releasedecayframes = 5;

/** Hold **/
int isholding = 0;  // Hold current note?

/** Note press data */
#define RELEASED 0
#define STILLRELEASED 1
#define PRESSED 2
#define STILLPRESSED 3
int curnotestate = STILLRELEASED;
int prevnotestate;
#define ISLEGATO ((curnotestate == PRESSED) && (prevnotestate == STILLPRESSED))
#define PLAYING ((curnotestate == PRESSED) || (curnotestate == STILLPRESSED))

// The keyboard spans a major tenth, C to E
#define KEYTABLE_SIZE 17
// The keytable keeps track of which keys are pressed. It is
// used to determine which note to play out, in the case that
// multiple keys were pressed.
short keytable[KEYTABLE_SIZE];
short keytable_prev[KEYTABLE_SIZE];

/** Vibrato **/
int prevvibstate;
int curvibstate = STILLRELEASED;
// The two parameters that control vibrato: frequency and length of
// pressing spacebar. The former controls speed and the latter controls
// depth.
int frames_onspace, frames_betweenspace;
float vibspeed = 0, vibdepth = 1;
float vibphase = 0;
#define PREVVIBPLAYING \
  ((prevvibstate == PRESSED) || (prevvibstate == STILLPRESSED))
#define VIBPLAYING ((curvibstate == PRESSED) || (curvibstate == STILLPRESSED))

/** Effects **/
#define MAXDIVEFRAMES (FPS / 2)
// The dive effect
int frames_dive = 0;

/** Text displays **/
const char* notetxt;
const char* octavetxt;
const char* voltxt;

/** Recording-related **/
// Are we recording?
int isrecording = 0;
// Struct to facilitate .wav output
TinyWav tw;

/** Mouse input **/
// At each frame, exactly one of the variables will be set
// depending on whether the vertical or horizontal motion
// of the mouse is larger.
float mousedx = 0;
float mousedy = 0;

Font font;

#endif
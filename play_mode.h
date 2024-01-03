#ifndef _PLAY_MODE
#define _PLAY_MODE

#include "raylib.h"
#include "globals.h"
#include "util.h"
#include "note.h"
#include "octave.h"
#include "synthesise.h"
#include "gui.h"
#include "sample.h"

// Number of seconds for the drawn wave to move one full cycle
#define WAVESPEED 2.0

void play_mode_gui();

#endif
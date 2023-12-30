#ifndef _PLAY_MODE
#define _PLAY_MODE

#include "raylib.h"
#include "globals.h"
#include "util.h"
#include "note.h"
#include "octave.h"
#include "synthesise.h"
#include "gui.h"

// Number of seconds for the drawn wave to move one full cycle
#define WAVESPEED 2.0

#define PLAY_MODE_BG (Color){64, 82, 74, 255}

void play_mode_gui();

#endif
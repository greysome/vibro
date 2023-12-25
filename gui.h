#ifndef GUI
#define GUI

#include "raylib.h"
#include "globals.h"
#include "util.h"
#include "note.h"
#include "octave.h"
#include "synthesise.h"

// Number of seconds for the drawn wave to move one full cycle
#define WAVESPEED 2.0

#define SUBGUI_BG (Color){60, 75, 65, 255}
#define BG (Color){64, 82, 74, 255}
#define SHADOW (Color){0, 0, 0, 200}

#define FONTSIZE 20
#define XMARGIN 40
#define YMARGIN 20

void display_note_text();
void display_octave_text();
void display_mode_text();
void draw_volume_level();
void draw_wave();

#endif
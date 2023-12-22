/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "globals.h"

int screenwidth;
int screenheight;

int wavetype = PULSE;

float addsynth_coeffs[NUM_HARMONICS] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

int isrecording = 0;
TinyWav tw;

float mousedx = 0;
float mousedy = 0;

Font font;

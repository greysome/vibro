#ifndef _INSTRUMENT_MODE
#define _INSTRUMENT_MODE

#include <assert.h>
#include <string.h>
#include "stb_ds.h"
#include "util.h"
#include "raylib.h"
#include "globals.h"
#include "gui.h"
#include "instrument.h"

void load_instrument_mode_state(int instrument_num);
void cleanup_instrument_mode_state();
void commit_instrument_mode_changes();
void reset_entryrow();
void instrument_mode_gui();

#endif
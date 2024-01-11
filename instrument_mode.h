#ifndef _INSTRUMENT_MODE
#define _INSTRUMENT_MODE

#include <assert.h>
#include <string.h>
#include "util.h"
#include "raylib.h"
#include "globals.h"
#include "gui.h"
#include "note.h"

#define MAX_STR_LEN 100
#define MULTISAMPLE_MAX 33

typedef enum {
  PULSE, TRI, SAW, SAMPLE, MULTISAMPLE
} WaveType;

typedef struct {
  int attack_frames;
  int decay_frames;
  float sustain_vol; // A proportion of note_vol
  int release_frames;
} ADSRParams;

typedef struct {
  /* User-specified settings */
  char path[MAX_STR_LEN];
  float pitch_modifier;
  float volume_modifier;
  bool play_continuously;
  bool stop_on_release;
  ADSRParams adsr;

  /* Internal state */
  bool is_ready;
  int sample_rate;
  int num_frames;
  bool is_alias;
  Sound sound;
  const float *data;
} Sample;

typedef struct {
  char name[MAX_STR_LEN];
  WaveType type;
  /* If type is MULTISAMPLE, then the ADSR parameters are stored
   * in the entries of `samples`. */
  ADSRParams adsr;

  float pulse_width;
  bool tri_nes_style;
  bool saw_nes_style;

  Sample samples[NOTETABLE_SIZE];
} Instrument;

int get_cur_instrument_idx();
Instrument get_cur_instrument();
int get_num_instruments();
void init_instrument(Instrument *instrument, int num);
void add_instrument();
void select_previous_instrument();
void select_next_instrument();
void load_instrument_mode_state(int instrument_num);
void cleanup_instrument(int instrument_num);
void cleanup_instruments();
void cleanup_instrument_mode_state();
void commit_instrument_mode_changes();
void reset_entryrow();
void instrument_mode_gui();

#endif
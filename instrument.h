#ifndef _INSTRUMENT
#define _INSTRUMENT

#include "raylib.h"
#include "note.h"

#define MAX_STR_LEN 100
#define NUM_HARMONICS 8

typedef enum {
  PULSE, TRI, SAW, SINE, SAMPLE, MULTISAMPLE
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
  float sine_coeffs[NUM_HARMONICS];

  Sample samples[NOTETABLE_SIZE];
} Instrument;

Instrument *get_instruments();
int get_cur_instrument_idx();
void increment_cur_instrument_idx();
Instrument *get_cur_instrument();
int get_num_instruments();
void init_instrument(Instrument *instrument, int num);
void add_instrument();
void delete_instrument(int instrument_num);
void select_previous_instrument();
void select_next_instrument();
void cleanup_instrument(int instrument_num);
void cleanup_instruments();

#endif
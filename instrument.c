#include "instrument.h"

#define STB_DS_IMPLEMENTATION
// Ignore GCC errors when compiling stb_ds.h
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-null-dereference"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-deref-before-check"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-null-argument"
#include "stb_ds.h"
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

static Instrument *instruments;
static int cur_instrument_idx = 0;

Instrument *get_instruments() {
  return instruments;
}

int get_cur_instrument_idx() {
  return cur_instrument_idx;
}

void increment_cur_instrument_idx() {
  cur_instrument_idx++;
}

int get_num_instruments() {
  return (int)arrlenu(instruments);
}

Instrument *get_cur_instrument() {
  assert(get_num_instruments() > cur_instrument_idx);
  return &instruments[cur_instrument_idx];
}

static void init_adsr_params(ADSRParams *adsr) {
  adsr->attack_frames = 10;
  adsr->decay_frames = 5;
  adsr->sustain_vol = 0.7;
  adsr->release_frames = 20;
}

static void init_sample_fields(Sample *sample) {
  memset(sample->path, 0, MAX_STR_LEN * sizeof(char));
  sample->pitch_modifier = 1;
  sample->volume_modifier = 1;
  sample->play_continuously = false;
  sample->stop_on_release = false;
  init_adsr_params(&sample->adsr);

  sample->is_ready = false;
  sample->sample_rate = NIL;
  sample->num_frames = NIL;

  sample->is_alias = false;
  memset(&sample->sound, 0, sizeof(Sound));
  sample->data = NULL;
}

void init_instrument(Instrument *instrument, int num) {
  strcpy(instrument->name, TextFormat("Instrument %d", num));
  instrument->type = PULSE;
  init_adsr_params(&instrument->adsr);
  instrument->pulse_width = 0.5;
  instrument->tri_nes_style = false;
  instrument->saw_nes_style = false;
  instrument->sine_coeffs[0] = 1;
  for (int i = 1; i < NUM_HARMONICS; i++)
    instrument->sine_coeffs[i] = 0;
  for (int i = 0; i < NOTETABLE_SIZE; i++)
    init_sample_fields(&instrument->samples[i]);
}

void add_instrument() {
  Instrument instrument;
  init_instrument(&instrument, get_num_instruments()+1);
  arrput(instruments, instrument);
}

void delete_instrument(int instrument_num) {
  assert(instrument_num >= 0 && instrument_num < get_num_instruments());
  cleanup_instrument(instrument_num);
  arrdel(instruments, instrument_num);
}

void select_previous_instrument() {
  cur_instrument_idx = clamp(cur_instrument_idx-1, 0, get_num_instruments()-1);
}

void select_next_instrument() {
  cur_instrument_idx = clamp(cur_instrument_idx+1, 0, get_num_instruments()-1);
}

void cleanup_instrument(int instrument_num) {
  assert(instrument_num >= 0 && instrument_num < get_num_instruments());
  Instrument instrument = instruments[instrument_num];
  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    Sample sample = instrument.samples[note];
    if (!sample.is_ready)
      continue;
    if (instrument.samples[note].is_alias)
      UnloadSoundAlias(instrument.samples[note].sound);
    else {
      UnloadSound(instrument.samples[note].sound);
      UnloadWaveSamples((float *)instrument.samples[note].data);
    }
    init_sample_fields(&instruments[instrument_num].samples[note]);
  }
}

void cleanup_instruments() {
  for (int i = 0; i < get_num_instruments(); i++)
    cleanup_instrument(i);
}
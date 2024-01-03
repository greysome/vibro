#include "sample.h"

static SamplePlaybackState playback_state;

static void reset_sample_playback_state() {
  playback_state.pitch_modifier = 1;
  playback_state.sample_frame_counter = NIL;
}

void play_sample() {
  Instrument instrument = get_instrument();
  if (!instrument.sample_ready)
    return;

  int note = get_cur_note();
  NoteState note_state = get_cur_note_state();

  float pitch_modifier = instrument.sample_pitch_modifier * powf(SEMITONE, note) * get_bend_modifier() * get_gliss_modifier() * get_vib_modifier() * get_dive_modifier();
  if (is_autoglissing())
    pitch_modifier = powf(SEMITONE, note) * get_autogliss_modifier();
  SetSoundPitch(instrument.sample, pitch_modifier);
  playback_state.pitch_modifier = pitch_modifier;

  SetSoundVolume(instrument.sample, get_actual_vols()[note]);

  if (note_state == PRESSED) {
    playback_state.sample_frame_counter = 0;
    PlaySound(instrument.sample);
  }
  else if (note_state == HELD) {
    if (IsSoundPlaying(instrument.sample))
      playback_state.sample_frame_counter += instrument.sample_rate * pitch_modifier / FPS;
    else if (instrument.sample_play_continuously) {
      playback_state.sample_frame_counter = 0;
      PlaySound(instrument.sample);
    }
  }
  else if (note_state == RELEASED && instrument.sample_stop_on_release && IsSoundPlaying(instrument.sample)) {
    reset_sample_playback_state();
    StopSound(instrument.sample);
  }
}

SamplePlaybackState get_sample_playback_state() {
  return playback_state;
}
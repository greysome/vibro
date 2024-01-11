#include "sample.h"

static SamplePlaybackState playback_states[NOTETABLE_SIZE];

static void reset_sample_playback_state(int note) {
  playback_states[note].pitch_modifier = 1;
  playback_states[note].sample_frame_counter = NIL;
}

void handle_sample_playback(int note, Sample sample) {
  float pitch_modifier = sample.pitch_modifier * powf(SEMITONE, note) * get_bend_modifier() * get_gliss_modifier() * get_vib_modifier() * get_dive_modifier();
  if (is_autoglissing())
    pitch_modifier = powf(SEMITONE, note) * get_autogliss_modifier();
  SetSoundPitch(sample.sound, pitch_modifier);
  playback_states[note].pitch_modifier = pitch_modifier;

  SetSoundVolume(sample.sound, sample.volume_modifier * get_actual_vols()[note]);

  int note_state = get_cur_note_states()[note];

  if (note_state == PRESSED) {
    playback_states[note].sample_frame_counter = 0;
    PlaySound(sample.sound);
  }
  else if (note_state == HELD) {
    if (IsSoundPlaying(sample.sound))
      playback_states[note].sample_frame_counter += sample.sample_rate * pitch_modifier / FPS;
    else if (sample.play_continuously) {
      playback_states[note].sample_frame_counter = 0;
      PlaySound(sample.sound);
    }
  }
  else if (note_state == RELEASED && sample.stop_on_release && IsSoundPlaying(sample.sound)) {
    reset_sample_playback_state(note);
    StopSound(sample.sound);
  }

  if ((note_state == RELEASED || note_state == STILLRELEASED) && !sample.stop_on_release)
    playback_states[note].sample_frame_counter += sample.sample_rate * pitch_modifier / FPS;
}

SamplePlaybackState *get_sample_playback_states() {
  return playback_states;
}
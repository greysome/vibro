#include "volume.h"

static float note_vol = 0.5;
// Updates frame-by-frame according to ADSR envelope
static float actual_vols[NOTETABLE_SIZE] = {0};

void update_note_vol() {
  note_vol += clamp(mouse_dx * 0.001, -0.01, 0.01);
  note_vol = clamp(note_vol, 0, 1);
  if (is_chord_mode()) return;

#define match_key_to_note_vol(key, vol) \
  if (IsKeyDown(key)) { note_vol = vol; }

  match_key_to_note_vol(KEY_ONE, 0.1);
  match_key_to_note_vol(KEY_TWO, 0.2);
  match_key_to_note_vol(KEY_THREE, 0.3);
  match_key_to_note_vol(KEY_FOUR, 0.4);
  match_key_to_note_vol(KEY_FIVE, 0.5);
  match_key_to_note_vol(KEY_SIX, 0.6);
  match_key_to_note_vol(KEY_SEVEN, 0.7);
  match_key_to_note_vol(KEY_EIGHT, 0.8);
  match_key_to_note_vol(KEY_NINE, 0.9);
  match_key_to_note_vol(KEY_ZERO, 1.0);
#undef match_key_to_note_vol
}

void apply_adsr() {
  static ADSRState adsr_states[NOTETABLE_SIZE] = {0};
  static int attack_frames = 1;
  static int decay_frames = 10;
  static float sustain_vol = 0.7; // A proportion of note_vol
  static int release_frames = 10;
  static float release_peaks[NOTETABLE_SIZE] = {0};  // Value of actualvol right when release starts

  for (int note = 0; note < NOTETABLE_SIZE; note++) {
    NoteState note_state = get_cur_note_states()[note];

    if (note_state == PRESSED) {
      adsr_states[note] = ATTACK;
      // It is possible to set this to 0 instead, but it results in unpleasant
      // popping noises when switching notes repeatedly
      actual_vols[note] = note_vol / (float)attack_frames;
    }
    else if (note_state == HELD) {
      if (adsr_states[note] == ATTACK) {
	actual_vols[note] += note_vol / (float)attack_frames;
	if (actual_vols[note] >= note_vol) {
	  adsr_states[note] = DECAY;
	  actual_vols[note] = note_vol;
	}
      }
      else if (adsr_states[note] == DECAY) {
	actual_vols[note] -= (note_vol - sustain_vol * note_vol) / (float)decay_frames;
	if (actual_vols[note] <= sustain_vol * note_vol) {
	  adsr_states[note] = SUSTAIN;
	  actual_vols[note] = sustain_vol * note_vol;
	}
      }
      else if (adsr_states[note] == SUSTAIN)  // Note that sustain_vol can change while note is being sustained
	actual_vols[note] = sustain_vol * note_vol;
      else if (adsr_states[note] == RELEASE) {  // Occurs during autogliss when note state is overriden from PRESSED to HELD
	adsr_states[note] = SUSTAIN;
	actual_vols[note] = sustain_vol * note_vol;
      }
    }
    else if (note_state == RELEASED) {
      adsr_states[note] = RELEASE;
      release_peaks[note] = actual_vols[note];
    }
    else if (note_state == STILLRELEASED) {
      if (actual_vols[note] > 0)
	actual_vols[note] -= release_peaks[note] / (float)release_frames;
      else {
	kill_note(note);
	actual_vols[note] = 0;
	release_peaks[note] = 0;
      }
    }
    else if (note_state == IDLE) {
      actual_vols[note] = 0;
    }
  }

  //printf("%d %d\n", get_prev_note_state(), get_cur_note_state());
  //printf("note states: "); for (int i = 0; i < 10; i++) printf("%d ", get_cur_note_states()[i]); printf("\n");
  //printf("adsr states: "); for (int i = 0; i < 10; i++) printf("%d ", adsr_states[i]); printf("\n");
  //printf("vols: "); for (int i = 0; i < 10; i++) printf("%f ", actual_vols[i]); printf("\n\n");
}

float get_note_vol() {
  return note_vol;
}

float *get_actual_vols() {
  return actual_vols;
}

bool is_silent() {
  for (int i = 0; i < NOTETABLE_SIZE; i++)
    if (actual_vols[i] > 0)
      return false;
  return true;
}
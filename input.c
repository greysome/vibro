/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include "input.h"

void process_keyboard_inputs() {
    // Local/global octave needs to be updated BEFORE note state.
    // This is because a change in octave will cause change in note state,
    // even if the same key is being held.
    update_global_octave();
    update_local_octave_modifier();

    if (is_chord_mode()) update_note_states();
    else update_note_state();

    update_pitch_bend();
    if (!is_chord_mode()) update_autogliss();
    update_gliss();
    update_effects();
    update_vib();

    update_sustain_vol();
    apply_adsr();
}
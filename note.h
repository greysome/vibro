#ifndef NOTE
#define NOTE

#include <string.h>
#include "raylib.h"
#include "globals.h"
#include "octave.h"

// The total number of notes that can be played via key presses
// NOTE: the QWERTY row and number row keys are only bound to notes in chord
// mode. In solo mode, they are not bound to anything, so the number of playable
// notes is actually 18.
#define NOTETABLE_SIZE 33

// A note is PRESSED on the very first frame the corresponding key is pressed,
// and RELEASED on the very first frame the corresponding key is released.
// STILLRELEASED means the ADSR envelope for that note is at the RELEASE stage
// (meaning it is still audible in the output), and IDLE means the note is
// completely silent.
typedef enum {
  PRESSED, HELD, RELEASED, STILLRELEASED, IDLE
} NoteState;

/** Common functions **/
/* NOTE: the ints that are returned refer to indices in the notetable.
   The first entry is a B, so it corresponds to 0.
   Thus, when converting these ints to frequencies based on C4, one has to first subtract by 1. */
int *get_cur_notes();
NoteState *get_cur_note_states();
int get_prev_note();
NoteState get_prev_note_state();
bool is_chord_mode();
bool is_solo_mode();
void toggle_chord_mode();
bool is_any_note_playing(); /* A note is playing if its state is either PRESSED or HELD. */
bool is_any_note_pressed();
void kill_note(int note); /* Set note's state to IDLE. */
void kill_notes(); /* Set every note's state to IDLE. */

/* (SOLO MODE)
   Gets the currently playing note, or NIL otherwise.
   This can also be used in chord mode, in which case it returns the
   first down note. In particular, this is useful when displaying the octave
   number. */
int get_cur_note();
/* (SOLO MODE)
   Gets the state of the current playing note, or NIL otherwise. */
NoteState get_cur_note_state();
/* (SOLO MODE)
   Returns true if a new note was pressed without releasing the previous note,
   i.e. the notes are being played continuously. */
bool is_legato();
/* (SOLO MODE)
   If the current note's state is PRESSED, immediately make it HELD instead.
   This has the effect of bypassing the ATTACK and DECAY stages of the ADSR
   envelope, and is utilised for autogliss. */
void no_attack();

void update_note_state(); /* Do the relevant note updates for that frame */

#endif
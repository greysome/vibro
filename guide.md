# General guide

This is a quick introduction to some concepts used in the codebase. It contains information useful for both users and developers.

### Notes (`note.c/note.h`)

A note is defined as a key that is bound to a sound.

There are 33 hardcoded notes in total, arranged in a piano layout:
```
1  2  3     5  6  7     9  0     =   |  higher
 Q  W  E  R  T  Y  U  I  O  P  [  ]  |  octave(s)
  A  S  D     G  H  J     L  ;          |  lower
   Z  X  C  V  B  N  M  ,  .  /         |  octave
```
`Z` is a low C, `Q` is a high C, and `I` is an even higher C. For convenience, I also added in the notes `A` and `1` for low B and high B.

Besides assigning notes to definite pitches, they can also be bound to .wav samples.

#### Solo/chord mode

In chord mode, multiple keys can be sounded out at once, allowing chords to be produced. Thus if I press `Z X C` simultaneously, a C major triad will really sound out. This is in contrast to solo mode, where only the most recently held key will sound out. So if I press down `Z X C` in that order, only a G will sound out.

Also in solo mode, the notes for the higher octaves are disabled, so the number of playable notes decreases to 18.

#### Note states

At each frame, a note is in one of five states: `PRESSED`, `HELD`, `RELEASED`, `STILLRELEASED` or `IDLE`. This is the `NoteState` enum. The 33 note states, stored in `cur_note_states`, are updated each frame.

When a key is pressed, the corresponding note becomes `PRESSED` for that single frame. For as long as the key is held down, the note remains `HELD`. When the key is finally released, it is `RELEASED` for that one frame. Then, as long as it remains released *and* the release envelope is still playing, the note is `STILLRELEASED`. (Thus the note is still audible even though the key is not pressed down, so notes and keys are not exactly the same.) After the release envelope, the note goes back to being `IDLE`, until it is pressed down again.

I use the phrase "note is playing" to mean the note is either in the `PRESSED` or `HELD` states. (It is crucial to note that the `RELEASED` and `STILLRELEASED` states are excluded.)


#### Current/previous notes

In solo mode, the **current note** is the currently playing one. It can be `NIL` if no notes are playing. The **previous note** is the note that was played in the previous frame, or `NIL`.

To illustrate, here's a sample sequence of events and the current/previous notes at those points:

|Event|Current note|Previous note|
|-----|------------|-------------|
|`Z` pressed|`Z`|NIL|
|`C` pressed while `Z` is held down|`C`|`Z`|
|`Z` released, `C` still held down|`C`|`C`|
|`C` released|NIL|`C`|
|`V` pressed|`V`|NIL|

In chord mode, the current note refers to one of the currently playing notes, namely the first such one in the `cur_note_states` list; it is `NIL` otherwise. The previous note is not relevant in chord mode.

#### Legato

Legato is when a new note is pressed while the previous note is still held. There is a function `is_legato()` to detect this, mainly to activate autogliss.

### Frequency (`freq.c/freq.h`)

As mentioned above, each note is bound to a pitch. However, the *actual* frequency being sounded out can be different, due to a variety of expressive effects:

1. Octave changes
2. Small pitch bends (via the mousewheel)
3. Larger pitch glisses (by moving the mouse up/down)
4. Vibrato (via the `SPC` key)
5. Pitch dives (via the `ALT` key)
6. Autogliss

Each effect has an associated modifier, which is a `float` representing a multiple of the original frequency. (For autogliss there is no variable explicitly storing the modifier, but it is an implicit value computed within functions.) The modifiers are updated every frame. For example, the vibrato modifier might rapidly oscillate between 0.9 and 1.1 (not exact values).

For each note on a given frame, the **note frequency** refers to the frequency sounded out if *only* octave changes are taken into account. This can be thought of as a 'base frequency' upon which effect modifiers are applied, resulting in the **actual frequency**.

#### Autogliss

When a new note is pressed legato whilst a pitch gliss is performed (via vertical mouse movement), a gliss will be automatically executed from the previous to the new note. The speed of autogliss is determined by the speed of the mouse. Starting from the previous actual frequency, the frequency increases by a fixed factor each frame until the note frequency of the new note is reached.

The code behind autogliss is found in `freq.c:update_autogliss()`.

### Octave (`octave.c/octave.h`)

There are two octave values stored as signed integers. **Global octave** is controlled by the up/down arrow keys, and it determines the general register of the instrument (e.g. bass vs lead). On the other hand, **local octave** is controlled by left and right clicking. It is meant for hitting notes that are slightly out of range otherwise.

Without any octave changes, `Z` plays a C4 and the `Q` plays a C5. Thus a note may have its own integer octave modifier, before applying local and global octave. The **actual octave** is then 4 plus the octave modifier plus the global and local octaves.

The difference between current/previous actual octave is the same as with current/previous note.

### Instruments (`instrument.c/instrument.h`)

An instrument is a choice of wave type plus some additional type-specific settings. For instance, the `pulse_width` setting is specific to pulse waves.

### Volume and ADSR (`volume.c/volume.h`)

**Note volume** is a global volume parameter which can be controlled by moving the mouse left or right, or via the number keys in solo mode. However, the **actual volume** of a note varies with time according to an ADSR envelope, with the note volume determining a 'baseline' for this envelope.

Each note has an associated `ADSRState` which is either `ATTACK`, `DECAY`, `SUSTAIN` or `RELEASE`. The specific shape of the envelope is governed by 4 parameters, stored in an `ADSRParams` struct (defined in `instrument.h`).

`attack_frames` and `decay_frames` give the number of frames to execute the attack and decay envelopes. The peak volume of attack is the note volume, and after decaying the actual volume settles at `sustain_vol` times the note volume. Finally `release_frames` give the number of frames to execute the release envelope.

### Samples (`instrument.c/instrument.h/sample.c/sample.h`)

Two types of samples are supported, roughly corresponding to the two instrument types "sample" and "multisample".

A "sample" supports one .wav sample, and each note will be bound to a pitch-shifted version. For example, if the sample is of a guitar plucking C, then `Z` will play a plucked C whereas `B` will play a plucked G. Thus "sample" are meant for pitched sounds.

A "multisample" supports multiple .wav samples, but each sample can only be bound to one note. This is useful for non-pitched sounds such as percussion. A standard use case would be for drumsets.

Samples and multisamples are implemented internally by an array of 33 `Sample` structs (defined in `instrument.h`), one for each note. When a note is pressed, the .wav sample to play is located in this array.

For "samples", rather than storing 33 copies of the sample data, a boolean `is_alias` is set to true to indicate that the data can be found elsewhere, namely in the first entry of the array.

#### Sample parameters

Each sample has a few adjustable parameters, which can be seen in the `Sample` struct. `pitch_modifier` and `volume_modifier` are self-explanatory.

`play_continuously` determines whether the sample should loop when the note is held for longer than the sample length. For percussive sounds like plucks and drums this is usually false.

`stop_on_release` determines whether the sample should cut off when the note is released, or whether it should play until the end. For percussive sounds this is likely to be false.
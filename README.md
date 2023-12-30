# jankboard

![screenshot](/screenshot.png)

An instrument controlled by computer keyboard and mouse. Demo [here](https://www.youtube.com/watch?v=IdXwC5mfPfI).

## Features

- Expressive features! (see [Controls](#controls))
- ~~Sound recording~~
- ~~ADSR envelope modification~~
- ~~Additive synthesis (under the sine instrument)~~
(The underlying functionality is already supported in the code, I just need to cook up a nice GUI for these)

## Controls

- `Z S X D C` ... `, L . ; /` are the notes
- ~~`- = BACKSPACE \` to switch between triangle, sawtooth, pulse and sine waves (also can be changed by clicking on icon)~~ (Outdated, these controls will come under the currently-in-construction instrument mode)
- `1 ... 0` for global volume control
- Move mouse left/right for local volume control
- Spam `SPACE` for vibrato. How long you press it controls depth, how fast you press it controls speed.
- Left/right click to temporarily adjust octave
- `UP/DOWN` to permanently adjust octave
- Scroll to pitch bend
- Move mouse up/down to gliss
    - Precise glissing: while holding down current note, press the note to gliss to *while* moving mouse up/down
- `CTRL` to toggle chord mode. Multiple notes can be played at once! (Note autogliss isn't supported)
- `ALT` for drop effect
- `TAB` to toggle fullscreen
- `` ` `` to record
- For right handers, I recommended the mouse be placed to the left of the keyboard.

## TODO

- [ ] FEATURE: instrument mode
- [ ] FEATURE: loop mode

## Notes
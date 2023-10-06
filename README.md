# jankboard

![screenshot](/screenshot.png)

An instrument controlled by computer keyboard and mouse. Demo [here](https://www.youtube.com/watch?v=IdXwC5mfPfI).

## Features

- Expressive features! (see [Controls](#controls))
- Sound recording
- ADSR envelope modification
- Additive synthesis (under the sine instrument)

## Controls

- `Z S X D C` ... `, L . ; /` are the notes
- `- = BACKSPACE \` to switch between triangle, sawtooth, pulse and sine waves (also can be changed by clicking on icon)
- `1 ... 0` for global volume control
- Move mouse left/right for local volume control
- Spam `SPACE` for vibrato. How long you press it controls depth, how fast you press it controls speed.
- Left/right click to temporarily adjust octave
- `UP/DOWN` to permanently adjust octave
- Scroll to pitch bend
- Move mouse up/down to gliss
    - Precise glissing: while holding down current note, press the note to gliss to *while* moving mouse up/down
- `ALT` for drop effect
- `CTRL` to sustain
- `CAPS LOCK` to toggle gliss lock (easier to change volume without changing pitch)
- `SHIFT` to enable constant volume (meaning each new note will start at the same global volume)
- `TAB` to toggle fullscreen
- `` ` `` to record
- For right handers, I recommended the mouse be placed to the left of the keyboard.

- Right shift to toggle showing cursor (which allows you to change controls)

## TODO

- [ ] BUG: Icons not showing when launched in another directory
- [ ] FEATURE: split build process for faster build. Right now I have a giant source file (keyboard.c) which simply includes everything else.

## Notes
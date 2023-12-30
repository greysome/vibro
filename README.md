# vibro

![Play mode](/screenshots/play_mode.png)
![Instrument mode](/screenshots/instrument_mode.png)

An instrument controlled by computer keyboard and mouse.

## Features

- Expressive features! (see [Controls](#controls))
- You can load in samples!
- ADSR envelopes!
- Sound recording! (doesn't work for samples unfortunately)
- Additive synthesis (under the sine instrument)

## Global controls

- `SHIFT-LEFT/RIGHT` to switch between instrument and note mode
- `TAB` to toggle fullscreen

## Play mode controls

- `LEFT/RIGHT` to change instrument

- `Z S X D C` ... `, L . ; /` are the notes
- `1 ... 0` for global volume control (only applicable for solo mode)
- Move mouse left/right for local volume control
- Spam `SPACE` for vibrato. How long you press it controls depth, how fast you press it controls speed.
- Left/right click to temporarily adjust octave
- `UP/DOWN` to permanently adjust octave
- Scroll to pitch bend
- Move mouse up/down to gliss
    - Precise glissing: while holding down current note, press the note to gliss to *while* moving mouse up/down
- `CTRL` to toggle chord mode. Multiple notes can be played at once! (Note autogliss isn't supported)
- `ALT` for drop effect
- `` ` `` to record
- For right handers, I recommended the mouse be placed to the left of the keyboard.

## Instrument mode controls

- Navigate using arrow keys
- For text fields: only `BACKSPACE` and `DEL` (to clear entire field) are supported
- For value fields (e.g. pitch modifier): use left/right arrow keys for coarse changes, mouse scrollwheel for fine changes
- Press `ENTER` to carry out certain options (like ADD/DELETE in the multisample submenu)

## Credits

I'd like to thank the following libraries for letting me focus on the application logic:

- Raylib by Ramon Santamaria
- stb_ds.h by Sean Barrett
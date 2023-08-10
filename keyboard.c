/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "globals.h"
#include "raylib.h"
#include "synthesise.h"
#include "tinywav/tinywav.h"
#include "util.h"

/**
  Functions to update the sound characteristics based on user
  input.
**/
void update_notevol() {
  sustainvol += clamp(mousedx * 0.001, -0.01, 0.01);
  sustainvol = clamp(sustainvol, 0, 1);

#define key2vol(key, vol) \
  if (IsKeyDown(key)) {   \
    sustainvol = vol;        \
    startnotevol = vol;   \
  }

  key2vol(KEY_ONE, 0.1);
  key2vol(KEY_TWO, 0.2);
  key2vol(KEY_THREE, 0.3);
  key2vol(KEY_FOUR, 0.4);
  key2vol(KEY_FIVE, 0.5);
  key2vol(KEY_SIX, 0.6);
  key2vol(KEY_SEVEN, 0.7);
  key2vol(KEY_EIGHT, 0.8);
  key2vol(KEY_NINE, 0.9);
  key2vol(KEY_ZERO, 1.0);
#undef key2vol

  if (curnotestate == PRESSED && isconstvol)
    sustainvol = startnotevol;
}

void update_actualvol() {
  if (PLAYING)
    actualvol = (frames_newnote <= FPS - 1) ? sustainvol * attackenvel[frames_newnote]
                                         : sustainvol;
  else
    actualvol = (frames_releasenote <= FPS - 1)
                 ? sustainvol * releaseenvel[frames_releasenote]
                 : 0;
}

void update_pitchbend() {
  if (curnotestate == PRESSED) {
    frames_noscroll = 0;
    isscrolling = 0;
    freq_bend_factor = 1;
  }
  float dy = GetMouseWheelMove();
  if (dy > 0) {
    freq_bend_factor *= pow(1.0194, dy);
    isscrolling = 1;
  } else if (dy < 0) {
    freq_bend_factor /= pow(1.0194, -dy);
    isscrolling = 1;
  } else
    frames_noscroll++;

  if (frames_noscroll >= 5) {
    frames_noscroll = 0;
    // Do pitch bend correction so that final note is not
    // out of tune
    if (isscrolling) {
      // Sorry for messy code I'll fix this later I promise
      // Upwards
      if (sqrt(SEMITONE) < freq_bend_factor &&
          freq_bend_factor < SEMITONE * sqrt(SEMITONE))
        freq_bend_factor = SEMITONE;
      else if (SEMITONE * sqrt(SEMITONE) <= freq_bend_factor &&
               freq_bend_factor < SEMITONE * SEMITONE * sqrt(SEMITONE))
        freq_bend_factor = SEMITONE * SEMITONE;
      else if (SEMITONE * SEMITONE * sqrt(SEMITONE) <= freq_bend_factor &&
               freq_bend_factor <
                   SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE))
        freq_bend_factor = SEMITONE * SEMITONE * SEMITONE;
      else if (SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE) <=
                   freq_bend_factor &&
               freq_bend_factor <
                   SEMITONE * SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE))
        freq_bend_factor = SEMITONE * SEMITONE * SEMITONE * SEMITONE;

      // Downwards
      else if (1.0 / (SEMITONE * sqrt(SEMITONE)) <= freq_bend_factor &&
               freq_bend_factor < 1.0 / sqrt(SEMITONE))
        freq_bend_factor = 1.0 / SEMITONE;
      else if (1.0 / (SEMITONE * SEMITONE * sqrt(SEMITONE)) <=
                   freq_bend_factor &&
               freq_bend_factor < 1.0 / (SEMITONE * sqrt(SEMITONE)))
        freq_bend_factor = 1.0 / (SEMITONE * SEMITONE);
      else if (1.0 / (SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE)) <=
                   freq_bend_factor &&
               freq_bend_factor < 1.0 / (SEMITONE * SEMITONE * sqrt(SEMITONE)))
        freq_bend_factor = 1.0 / (SEMITONE * SEMITONE * SEMITONE);
      else if (1.0 / (SEMITONE * SEMITONE * SEMITONE * SEMITONE *
                      sqrt(SEMITONE)) <=
                   freq_bend_factor &&
               freq_bend_factor <
                   1.0 / (SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE)))
        freq_bend_factor = 1.0 / (SEMITONE * SEMITONE * SEMITONE * SEMITONE);

      else
        freq_bend_factor = 1.0;
    }
  }
}

void update_gliss() {
  if (curnotestate == PRESSED)
    freq_gliss_factor = 1;
  if (isglisslock)
    return;
  float factor = 1.002;
  if (mousedy > 0)
    freq_gliss_factor /= pow(factor, mousedy);
  else
    freq_gliss_factor *= pow(factor, -mousedy);
  freq_gliss_factor = clamp(freq_gliss_factor, 0.5, 2);
}

void update_effects() {
  if (IsKeyPressed(KEY_LEFT_ALT) || IsKeyPressed(KEY_RIGHT_ALT))
    frames_dive = 1;

  if (frames_dive > 0) {
    if (++frames_dive <= MAXDIVEFRAMES)
      if (curnotestate == PRESSED) {
        frames_dive = 0;
        freq_dive_factor = 1;
      } else
        freq_dive_factor *= 0.95;
    else {
      curnotestate = RELEASED;
      frames_dive = 0;
      freq_dive_factor = 1;
    }
  }
}

void update_vib() {
  // Momentarily kill vibrato when a new note is pressed
  if (curnotestate == PRESSED) {
    vibdepth = 1;
    curvibstate = STILLRELEASED;
    frames_onspace = 0;
    frames_betweenspace = 0;
    freq_vib_factor = 1;
    return;
  }

  vibphase += vibspeed / FPS;
  prevvibstate = curvibstate;

  if (IsKeyDown(KEY_SPACE))
    curvibstate = VIBPLAYING ? STILLPRESSED : PRESSED;
  else
    curvibstate = VIBPLAYING ? RELEASED : STILLRELEASED;

  if (!PREVVIBPLAYING && VIBPLAYING) {
    vibspeed = 1.0 / frames_betweenspace;
    frames_betweenspace = 0;
  } else {
    if (frames_betweenspace > 30) {
      // Make vibrato decay if not 'replenished'
      vibspeed *= 0.8;
      vibphase *= 0.8;
    }
    frames_betweenspace++;
  }

  if (PREVVIBPLAYING && !VIBPLAYING) {
    vibdepth = clamp(pow(1.003, frames_onspace), 1, 1.04);
    frames_onspace = 0;
  }
  if (VIBPLAYING)
    frames_onspace++;

  vibphase += vibspeed;
  if (vibphase >= 1)
    vibphase = 0;
  freq_vib_factor = pow(vibdepth, sinf(vibphase * 2 * PI));
}

void update_pulsewidth() {
  curpulsewidth = pulsewidthenvel[min(frames_newnote, 2)];
}

void update_wavetype() {
  if (IsKeyPressed(KEY_MINUS))
    wavetype = TRI;
  if (IsKeyPressed(KEY_EQUAL))
    wavetype = SAW;
  if (IsKeyPressed(KEY_BACKSPACE))
    wavetype = PULSE;
}

void update_octave() {
  if (issustain && curnotestate == STILLPRESSED)
    return;
  if (IsKeyPressed(KEY_DOWN))
    globaloctave = clamp(globaloctave - 1, MINOCTAVE, MAXOCTAVE);
  if (IsKeyPressed(KEY_UP))
    globaloctave = clamp(globaloctave + 1, MINOCTAVE, MAXOCTAVE);

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && globaloctave > MINOCTAVE)
    octaveoffset = -1;
  else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && globaloctave < MAXOCTAVE)
    octaveoffset = 1;
  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
      !IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    octaveoffset = 0;

  actualoctave = globaloctave + octaveoffset;
}

void update_keytables() {
  for (int i = 0; i < KEYTABLE_SIZE; i++)
    keytable_prev[i] = keytable[i];

#define key2note(key, note) keytable[note] = IsKeyDown(key)
  key2note(KEY_Z, 0);
  key2note(KEY_S, 1);
  key2note(KEY_X, 2);
  key2note(KEY_D, 3);
  key2note(KEY_C, 4);
  key2note(KEY_V, 5);
  key2note(KEY_G, 6);
  key2note(KEY_B, 7);
  key2note(KEY_H, 8);
  key2note(KEY_N, 9);
  key2note(KEY_J, 10);
  key2note(KEY_M, 11);
  key2note(KEY_COMMA, 12);
  key2note(KEY_L, 13);
  key2note(KEY_PERIOD, 14);
  key2note(KEY_SEMICOLON, 15);
  key2note(KEY_SLASH, 16);
#undef key2note
}

void update_notestates() {
  prevnotestate = curnotestate;
  // Any change to the keytable?
  int changed = 0;
  // A note that has remained pressed, to use as current note
  // in case some other note is released.
  // Otherwise, x = -1 indicates a note release, unless sustain
  // is on.
  int x = -1;
  for (int i = 0; i < KEYTABLE_SIZE; i++) {
    if (keytable_prev[i] == 1 && keytable[i] == 1)
      x = i;
    // If a new note has been pressed, use that
    else if (keytable_prev[i] == 0 && keytable[i] == 1) {
      changed = 1;
      x = i;
      break;
    } else if (keytable_prev[i] == 1 && keytable[i] == 0)
      changed = 1;
  }

  if (IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL))
    issustain = !issustain;

  if (changed) {
    if (x == -1)
      curnotestate = issustain ? STILLPRESSED : RELEASED;
    else {
      curnotestate = PRESSED;
      curnote = x;
    }
  } else if (!issustain)  // If issustain, maintain previous state...
    curnotestate = (x == -1) ? STILLRELEASED : STILLPRESSED;
  else if (issustain &&
           (curnotestate == PRESSED))  // ...unless note was just pressed
    curnotestate = STILLPRESSED;

  // Update frames
  if (curnotestate == PRESSED)
    frames_newnote = 0;
  else
    frames_newnote++;

  if (curnotestate != STILLRELEASED)
    frames_releasenote = 0;
  else
    frames_releasenote++;
}

/** Drawing functions. **/
void setdisplaytxt() {
  int abs_note = curnote + actualoctave * 12;
  int scaledegree = mod(abs_note, 12);

#define scaledeg2txt(x, txt)                                           \
  if (scaledegree == x) {                                              \
    notetxt = TextFormat(txt "%d", actualoctave + (curnote / 12) + 4); \
  }

  scaledeg2txt(0, "C-");
  scaledeg2txt(1, "C#");
  scaledeg2txt(2, "D-");
  scaledeg2txt(3, "D#");
  scaledeg2txt(4, "E-");
  scaledeg2txt(5, "F-");
  scaledeg2txt(6, "F#");
  scaledeg2txt(7, "G-");
  scaledeg2txt(8, "G#");
  scaledeg2txt(9, "A-");
  scaledeg2txt(10, "A#");
  scaledeg2txt(11, "B-");
#undef scaledeg2txt

  if (octaveoffset == 1)
    octavetxt = TextFormat("OCTAVE %d+1", globaloctave + 4);
  else if (octaveoffset == -1)
    octavetxt = TextFormat("OCTAVE %d-1", globaloctave + 4);
  else
    octavetxt = TextFormat("OCTAVE %d", globaloctave + 4);

  voltxt = TextFormat("VOL %d%%", (int)(sustainvol * 100));
}

void drawwave() {
  if (actualvol == 0) {
    Vector2 start = {0, screenheight / 2};
    Vector2 end = {screenwidth, screenheight / 2};
    DrawLineEx(start, end, 2, WHITE);
    return;
  }

  float dphase = ACTUALFREQ / WAVEXSCALE;
  float phase = fmodf2((float)GetTime(), WAVESPEED) / WAVESPEED;
  int y, y_;
  for (int i = 0; i < screenwidth; i += 2) {
    phase += dphase;
    float nextphase = phase + dphase;
    if (nextphase >= 1)
      nextphase -= 1;
    if (wavetype == TRI) {
      y = screenheight / 2 - 100 * actualvol * nes_tri(phase);
      y_ = screenheight / 2 - 100 * actualvol * nes_tri(nextphase);
    } else if (wavetype == SAW) {
      y = screenheight / 2 - 100 * actualvol * nes_saw(phase);
      y_ = screenheight / 2 - 100 * actualvol * nes_saw(nextphase);
    } else if (wavetype == PULSE) {
      y = screenheight / 2 - 100 * actualvol * nes_pulse(phase);
      y_ = screenheight / 2 - 100 * actualvol * nes_pulse(nextphase);
    }
    Vector2 start = {i, y};
    Vector2 end = {i + 2, y_};
    DrawLineEx(start, end, 2, WHITE);
    if (phase >= 1)
      phase = fmodf(phase, 1.0);
  }
}

// Draw text with anchors at different corners
#define DrawTextLL(text, x, _y, fontsize, color)                             \
  DrawText(text, x, _y - MeasureTextEx(font, text, fontsize, 0).y, fontsize, \
           color)
#define DrawTextLR(text, x, _y, fontsize, color)  \
  DrawText(text, x - MeasureText(text, fontsize), \
           _y - MeasureTextEx(font, text, fontsize, 0).y, fontsize, color)
#define DrawTextUR(text, x, y, fontsize, color) \
  DrawText(text, x - MeasureText(text, fontsize), y, fontsize, color)

void draw() {
  ClearBackground(DARKGRAY);
  if (wavetype == TRI)
    DrawText("NES TRI", XMARGIN, YMARGIN, FONTSIZE, WHITE);
  else if (wavetype == SAW)
    DrawText("NES SAW", XMARGIN, YMARGIN, FONTSIZE, WHITE);
  else if (wavetype == PULSE)
    DrawText("PULSE", XMARGIN, YMARGIN, FONTSIZE, WHITE);

  setdisplaytxt();
  DrawText(octavetxt, XMARGIN, YMARGIN + YSPACE, FONTSIZE, WHITE);
  if (PLAYING)
    DrawText(notetxt, XMARGIN, YMARGIN + 2 * YSPACE, FONTSIZE, WHITE);
  DrawTextLL(voltxt, XMARGIN, screenheight - YMARGIN, FONTSIZE, WHITE);

  if (issustain)
    DrawTextLL("SUSTAIN", XMARGIN, screenheight - YMARGIN - YSPACE, FONTSIZE,
               WHITE);
  if (isglisslock)
    DrawTextLR("GLISS LOCK", screenwidth - XMARGIN, screenheight - YMARGIN,
               FONTSIZE, WHITE);
  if (isconstvol)
    DrawTextLR("CONST VOL", screenwidth - XMARGIN,
               screenheight - YMARGIN - YSPACE, FONTSIZE, WHITE);
  if (isrecording)
    DrawTextUR("RECORDING", screenwidth - XMARGIN, YMARGIN, FONTSIZE, WHITE);

  drawwave();
}

int main() {
  InitWindow(STARTINGWIDTH, STARTINGHEIGHT, "jankboard");
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAXSAMPLES_PER_UPDATE);

  AudioStream stream = LoadAudioStream(SAMPLERATE, BITDEPTH, 1);
  SetAudioStreamCallback(stream, synthesise);
  PlayAudioStream(stream);

  DisableCursor();
  SetTargetFPS(FPS);
  font = GetFontDefault();

  while (!WindowShouldClose()) {
    screenwidth = GetScreenWidth();
    screenheight = GetScreenHeight();

    mousedx = GetMouseDelta().x;
    mousedy = GetMouseDelta().y;
    if (abs(mousedx) >= abs(mousedy))
      mousedy = 0;
    else
      mousedx = 0;

    update_wavetype();
    update_keytables();
    update_notestates();
    update_octave();
    update_pitchbend();
    update_gliss();
    update_effects();
    update_vib();
    update_notevol();
    update_actualvol();
    update_pulsewidth();

    if (IsKeyPressed(KEY_LEFT_SHIFT))
      isconstvol = !isconstvol;

    if (IsKeyPressed(KEY_CAPS_LOCK))
      isglisslock = !isglisslock;

    if (IsKeyPressed(KEY_GRAVE)) {
      if (!isrecording)
        tinywav_open_write(&tw,
                           1,  // number of channels
                           SAMPLERATE, TW_INT16, TW_INLINE, RECORDFILE);
      else
        tinywav_close_write(&tw);
      isrecording = !isrecording;
    }

    if (IsKeyPressed(KEY_TAB)) {
      if (IsWindowFullscreen()) {
        ToggleFullscreen();
        SetWindowSize(STARTINGWIDTH, STARTINGHEIGHT);
      } else {
        SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
        ToggleFullscreen();
      }
    }

#define set_pulsewidthenvel(a, b, c) \
  {                                  \
    pulsewidthenvel[0] = a;          \
    pulsewidthenvel[1] = b;          \
    pulsewidthenvel[2] = c;          \
  }

    if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_P)) {
      set_pulsewidthenvel(0.5, 0.5, 0.5);
    } else if (IsKeyPressed(KEY_W)) {
      set_pulsewidthenvel(0.25, 0.25, 0.25);
    } else if (IsKeyPressed(KEY_E)) {
      set_pulsewidthenvel(0.125, 0.125, 0.125);
    }
#undef set_pulsewidthenvel

    BeginDrawing();
    draw();
    EndDrawing();
  }

  if (isrecording)
    tinywav_close_write(&tw);
  CloseWindow();
  UnloadAudioStream(stream);
  CloseAudioDevice();

  return 0;
}
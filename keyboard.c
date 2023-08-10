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
  notevol += clamp(mousedx * 0.001, -0.01, 0.01);
  notevol = clamp(notevol, 0, 1);

#define key2vol(key, vol) \
  if (IsKeyDown(key)) {   \
    notevol = vol;        \
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

  if (curstate == PRESSED && constvol)
    notevol = startnotevol;
}

void update_curvol() {
  if (PLAYING)
    curvol = (frames_newnote <= FPS - 1) ? notevol * attackenvel[frames_newnote]
                                         : notevol;
  else
    curvol = (frames_releasenote <= FPS - 1)
                 ? notevol * releaseenvel[frames_releasenote]
                 : 0;
}

void update_pitchbend() {
  if (curstate == PRESSED) {
    frames_noscroll = 0;
    scrolled = 0;
    freqoffset = 1;
  }
  float dy = GetMouseWheelMove();
  if (dy > 0) {
    freqoffset *= pow(1.0194, dy);
    scrolled = 1;
  } else if (dy < 0) {
    freqoffset /= pow(1.0194, -dy);
    scrolled = 1;
  } else
    frames_noscroll++;

  if (frames_noscroll >= 5) {
    frames_noscroll = 0;
    // Do pitch bend correction so that final note is not
    // out of tune
    if (scrolled) {
      // Sorry for messy code I'll fix this later I promise
      // Upwards
      if (sqrt(SEMITONE) < freqoffset && freqoffset < SEMITONE * sqrt(SEMITONE))
        freqoffset = SEMITONE;
      else if (SEMITONE * sqrt(SEMITONE) <= freqoffset &&
               freqoffset < SEMITONE * SEMITONE * sqrt(SEMITONE))
        freqoffset = SEMITONE * SEMITONE;
      else if (SEMITONE * SEMITONE * sqrt(SEMITONE) <= freqoffset &&
               freqoffset < SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE))
        freqoffset = SEMITONE * SEMITONE * SEMITONE;
      else if (SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE) <= freqoffset &&
               freqoffset <
                   SEMITONE * SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE))
        freqoffset = SEMITONE * SEMITONE * SEMITONE * SEMITONE;

      // Downwards
      else if (1.0 / (SEMITONE * sqrt(SEMITONE)) <= freqoffset &&
               freqoffset < 1.0 / sqrt(SEMITONE))
        freqoffset = 1.0 / SEMITONE;
      else if (1.0 / (SEMITONE * SEMITONE * sqrt(SEMITONE)) <= freqoffset &&
               freqoffset < 1.0 / (SEMITONE * sqrt(SEMITONE)))
        freqoffset = 1.0 / (SEMITONE * SEMITONE);
      else if (1.0 / (SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE)) <=
                   freqoffset &&
               freqoffset < 1.0 / (SEMITONE * SEMITONE * sqrt(SEMITONE)))
        freqoffset = 1.0 / (SEMITONE * SEMITONE * SEMITONE);
      else if (1.0 / (SEMITONE * SEMITONE * SEMITONE * SEMITONE *
                      sqrt(SEMITONE)) <=
                   freqoffset &&
               freqoffset <
                   1.0 / (SEMITONE * SEMITONE * SEMITONE * sqrt(SEMITONE)))
        freqoffset = 1.0 / (SEMITONE * SEMITONE * SEMITONE * SEMITONE);

      else
        freqoffset = 1.0;
    }
  }
}

void update_gliss() {
  if (curstate == PRESSED)
    freqoffset2 = 1;
  if (glisslock)
    return;
  float factor = 1.002;
  if (mousedy > 0)
    freqoffset2 /= pow(factor, mousedy);
  else
    freqoffset2 *= pow(factor, -mousedy);
  freqoffset2 = clamp(freqoffset2, 0.5, 2);
}

void update_effects() {
  if (IsKeyPressed(KEY_LEFT_ALT) || IsKeyPressed(KEY_RIGHT_ALT))
    frames_drop = 1;

  if (frames_drop > 0) {
    if (++frames_drop <= MAXDROPFRAMES)
      if (curstate == PRESSED) {
        frames_drop = 0;
        freqoffset_drop = 1;
      } else
        freqoffset_drop *= 0.95;
    else {
      curstate = RELEASED;
      frames_drop = 0;
      freqoffset_drop = 1;
    }
  }
}

void update_vib() {
  // Momentarily kill vibrato when a new note is pressed
  if (curstate == PRESSED) {
    vibdepth = 1;
    curvibstate = STILLRELEASED;
    frames_onspace = 0;
    frames_betweenspace = 0;
    freqoffset3 = 1;
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
  freqoffset3 = pow(vibdepth, sinf(vibphase * 2 * PI));
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
  if (sustain && curstate == STILLPRESSED)
    return;
  if (IsKeyPressed(KEY_DOWN))
    preoctave = clamp(preoctave - 1, MINOCTAVE, MAXOCTAVE);
  if (IsKeyPressed(KEY_UP))
    preoctave = clamp(preoctave + 1, MINOCTAVE, MAXOCTAVE);

  if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && preoctave > MINOCTAVE)
    octaveoffset = -1;
  else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && preoctave < MAXOCTAVE)
    octaveoffset = 1;
  if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
      !IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    octaveoffset = 0;

  octave = preoctave + octaveoffset;
}

void update_notetables() {
  for (int i = 0; i < NOTETABLE_SIZE; i++)
    notetable_prev[i] = notetable[i];

#define key2note(key, note) notetable[note] = IsKeyDown(key)
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
  prevstate = curstate;
  // Any change to the notetable?
  int changed = 0;
  // A note that has remained pressed, to use as current note
  // in case some other note is released.
  // Otherwise, x = -1 indicates a note release, unless sustain
  // is on.
  int x = -1;
  for (int i = 0; i < NOTETABLE_SIZE; i++) {
    if (notetable_prev[i] == 1 && notetable[i] == 1)
      x = i;
    // If a new note has been pressed, use that
    else if (notetable_prev[i] == 0 && notetable[i] == 1) {
      changed = 1;
      x = i;
      break;
    } else if (notetable_prev[i] == 1 && notetable[i] == 0)
      changed = 1;
  }

  if (IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL))
    sustain = !sustain;

  if (changed) {
    if (x == -1)
      curstate = sustain ? STILLPRESSED : RELEASED;
    else {
      curstate = PRESSED;
      curnote = x;
    }
  } else if (!sustain)  // If sustain, maintain previous state...
    curstate = (x == -1) ? STILLRELEASED : STILLPRESSED;
  else if (sustain && (curstate == PRESSED))  // ...unless note was just pressed
    curstate = STILLPRESSED;

  // Update frames
  if (curstate == PRESSED)
    frames_newnote = 0;
  else
    frames_newnote++;

  if (curstate != STILLRELEASED)
    frames_releasenote = 0;
  else
    frames_releasenote++;
}

/** Drawing functions. **/
void setdisplaytxt() {
  int abs_note = curnote + octave * 12;
  int scaledegree = mod(abs_note, 12);

#define scaledeg2txt(x, txt)                                     \
  if (scaledegree == x) {                                        \
    notetxt = TextFormat(txt "%d", octave + (curnote / 12) + 4); \
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
    octavetxt = TextFormat("OCTAVE %d+1", preoctave + 4);
  else if (octaveoffset == -1)
    octavetxt = TextFormat("OCTAVE %d-1", preoctave + 4);
  else
    octavetxt = TextFormat("OCTAVE %d", preoctave + 4);

  voltxt = TextFormat("VOL %d%%", (int)(notevol * 100));
}

void drawwave() {
  if (curvol == 0) {
    Vector2 start = {0, HEIGHT / 2};
    Vector2 end = {WIDTH, HEIGHT / 2};
    DrawLineEx(start, end, 2, WHITE);
    return;
  }

  float dphase = CURFREQ / WAVEXSCALE;
  float phase = fmodf2((float)GetTime(), WAVESPEED) / WAVESPEED;
  int y, y_;
  for (int i = 0; i < WIDTH; i += 2) {
    phase += dphase;
    float nextphase = phase + dphase;
    if (nextphase >= 1)
      nextphase -= 1;
    if (wavetype == TRI) {
      y = HEIGHT / 2 - 100 * curvol * nes_tri(phase);
      y_ = HEIGHT / 2 - 100 * curvol * nes_tri(nextphase);
    } else if (wavetype == SAW) {
      y = HEIGHT / 2 - 100 * curvol * nes_saw(phase);
      y_ = HEIGHT / 2 - 100 * curvol * nes_saw(nextphase);
    } else if (wavetype == PULSE) {
      y = HEIGHT / 2 - 100 * curvol * nes_pulse(phase);
      y_ = HEIGHT / 2 - 100 * curvol * nes_pulse(nextphase);
    }
    Vector2 start = {i, y};
    Vector2 end = {i + 2, y_};
    DrawLineEx(start, end, 2, WHITE);
    if (phase >= 1)
      phase = fmodf(phase, 1.0);
  }
}

#define FONTSIZE 40
#define XMARGIN 40
#define YMARGIN 40
#define YSPACE 50
#define DEFAULTFONT GetFontDefault()
// Draw text with anchors at different corners
#define DrawTextLL(text, x, _y, fontsize, color)                          \
  DrawText(text, x, _y - MeasureTextEx(DEFAULTFONT, text, fontsize, 0).y, \
           fontsize, color)
#define DrawTextLR(text, x, _y, fontsize, color)                           \
  DrawText(text, x - MeasureText(text, fontsize),                          \
           _y - MeasureTextEx(DEFAULTFONT, text, fontsize, 0).y, fontsize, \
           color)
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
  DrawTextLL(voltxt, XMARGIN, HEIGHT - YMARGIN, FONTSIZE, WHITE);

  if (sustain)
    DrawTextLL("SUSTAIN", XMARGIN, HEIGHT - YMARGIN - YSPACE, FONTSIZE, WHITE);
  if (glisslock)
    DrawTextLR("GLISS LOCK", WIDTH - XMARGIN, HEIGHT - YMARGIN, FONTSIZE,
               WHITE);
  if (constvol)
    DrawTextLR("CONST VOL", WIDTH - XMARGIN, HEIGHT - YMARGIN - YSPACE,
               FONTSIZE, WHITE);
  if (recording)
    DrawTextUR("RECORDING", WIDTH - XMARGIN, YMARGIN, FONTSIZE, WHITE);

  drawwave();
}

int main() {
  InitWindow(NORMALWIDTH, NORMALHEIGHT, "jankboard");
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAXSAMPLES_PER_UPDATE);

  AudioStream stream = LoadAudioStream(SAMPLERATE, BITDEPTH, 1);
  SetAudioStreamCallback(stream, synthesise);
  PlayAudioStream(stream);

  DisableCursor();
  SetTargetFPS(FPS);
  while (!WindowShouldClose()) {
    mousedx = GetMouseDelta().x;
    mousedy = GetMouseDelta().y;
    if (abs(mousedx) >= abs(mousedy))
      mousedy = 0;
    else
      mousedx = 0;

    update_wavetype();
    update_notetables();
    update_notestates();
    update_octave();
    update_pitchbend();
    update_gliss();
    update_effects();
    update_vib();
    update_notevol();
    update_curvol();
    update_pulsewidth();

    if (IsKeyPressed(KEY_LEFT_SHIFT))
      constvol = !constvol;

    if (IsKeyPressed(KEY_CAPS_LOCK))
      glisslock = !glisslock;

    if (IsKeyPressed(KEY_GRAVE)) {
      if (!recording)
        tinywav_open_write(&tw,
                           1,  // number of channels
                           SAMPLERATE, TW_INT16, TW_INLINE, RECORDFILE);
      else
        tinywav_close_write(&tw);
      recording = !recording;
    }

    if (IsKeyPressed(KEY_TAB)) {
      if (IsWindowFullscreen()) {
        ToggleFullscreen();
        SetWindowSize(NORMALWIDTH, NORMALHEIGHT);
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

  if (recording)
    tinywav_close_write(&tw);
  CloseWindow();
  UnloadAudioStream(stream);
  CloseAudioDevice();

  return 0;
}
/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "adsr.h"
#include "debug.h"
#include "globals.h"
#include "raylib.h"
#include "synthesise.h"
#include "tinywav/tinywav.h"
#include "util.h"

/**
  Functions to update the sound characteristics based on user
  input.
**/
void update_sustainvol() {
  sustainvol += clamp(mousedx * 0.001, -0.01, 0.01);
  sustainvol = clamp(sustainvol, 0, 1);

#define key2vol(key, vol) \
  if (IsKeyDown(key)) {   \
    sustainvol = vol;     \
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

/* Apply ADSR envelope to obtain actualvol */
void update_actualvol() {
  if (curnotestate == PRESSED) {
    attackpeakvol = attackpeak * sustainvol;
    frames_into_sustain = 0;
    ADSRstate = ATTACK;
    // It is possible to set this to 0 instead, but it results in unpleasant
    // popping noises when switching notes repeatedly
    actualvol = attackpeakvol / (float)attackframes;
  } else if (curnotestate == STILLPRESSED) {
    if (ADSRstate == ATTACK) {
      actualvol += attackpeakvol / (float)attackframes;
      if (actualvol >= attackpeakvol) {
        ADSRstate = DECAY;
        actualvol = attackpeakvol;
      }
    } else if (ADSRstate == DECAY) {
      actualvol -= (attackpeakvol - sustainvol) / (float)decayframes;
      if (actualvol <= sustainvol) {
        ADSRstate = SUSTAIN;
        actualvol = sustainvol;
      }
    } else if (ADSRstate == SUSTAIN) {
      idebug(sustainvol, sustaindecayframes);
      frames_into_sustain++;
      actualvol =
          sustainvol * (1.0 - frames_into_sustain / (float)sustaindecayframes);
      if (actualvol <= 0) {
        ADSRstate = RELEASE;
        actualvol = 0;
      }
      // actualvol =
      //     sustainvol * 1.0 /
      //     (1 + expf(7.0 * frames_into_sustain / (float)sustaindecayframes -
      //     4));
    }
  } else if (curnotestate == RELEASED) {
    ADSRstate = RELEASE;
    releasepeak = actualvol;
  } else if (curnotestate == STILLRELEASED) {
    if (actualvol > 0)
      actualvol -= releasepeak / (float)releasedecayframes;
    else
      actualvol = 0;
  }
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
  prevmousedy = mousedy;
  if (curnotestate == PRESSED)
    freq_gliss_factor = 1;
  if (isglisslock)
    return;
  float factor = 1.0002;
  if (mousedy > 0)
    freq_gliss_factor /= pow(factor, mousedy);
  else if (mousedy < 0)
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

  // Just pressed space
  if (!PREVVIBPLAYING && VIBPLAYING) {
    vibspeed = 1.0 / frames_betweenspace;
    frames_betweenspace = 0;
  } else {
    frames_betweenspace++;
    if (frames_betweenspace > 30) {
      // Make vibrato decay if not 'replenished'
      vibspeed = 0;
      vibphase = 0;
      freq_vib_factor += 0.2 * (1.0 - freq_vib_factor);
      return;
    }
  }

  // Just released space
  if (PREVVIBPLAYING && !VIBPLAYING) {
    vibdepth = clamp(pow(1.003, frames_onspace), 1, 1.04);
    frames_onspace = 0;
  }
  // Still pressing on space
  if (VIBPLAYING)
    frames_onspace++;

  vibphase += vibspeed;
  if (vibphase >= 1)
    vibphase = 0;
  freq_vib_factor = pow(vibdepth, sinf(vibphase * 2 * PI));
}

void update_actualfreq() {
  // Autogliss is only activated when new note is pressed while glissing on the
  // previous note
  if (ISLEGATO && prevmousedy) {
    // We don't want to attack the new note
    curnotestate = STILLPRESSED;
    // Reset gliss; moving mouse vertically now does nothing until autogliss is
    // complete
    freq_gliss_factor = 1;
    autogliss_startfreq = actualfreq;
    // The frequency of the new note (without extras like pitch bend, vibrato,
    // etc.)
    autogliss_endfreq = C4 * pow(2, actualoctave) * pow(SEMITONE, curnote);

    frames_toautogliss =
        -abs(autogliss_endfreq - autogliss_startfreq) / prevmousedy * 1.3;
    frames_toautogliss = clamp(frames_toautogliss, 10, 20);

    float frac = autogliss_endfreq / autogliss_startfreq;
    autogliss_freqstep = powf(frac, 1.0 / frames_toautogliss);
  }
  // If currently autoglissing
  if (frames_toautogliss && curnotestate != RELEASED) {
    actualfreq *= autogliss_freqstep;
    frames_toautogliss--;
  } else {
    frames_toautogliss = 0;
    actualfreq =
        (C4 * pow(2, actualoctave) * pow(SEMITONE, curnote) * freq_bend_factor *
         freq_gliss_factor * freq_vib_factor * freq_dive_factor);
  }
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
  prevactualoctave = actualoctave;
  if (isholding && curnotestate == STILLPRESSED)
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
  // TODO: update comments
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
  changed |= prevactualoctave != actualoctave;

  if (IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL))
    isholding = !isholding;

  if (changed) {
    if (x == -1)
      curnotestate = isholding ? STILLPRESSED : RELEASED;
    else {
      curnotestate = PRESSED;
      curnote = x;
    }
  } else if (!isholding)  // If isholding, maintain previous state...
    curnotestate = (x == -1) ? STILLRELEASED : STILLPRESSED;
  else if (isholding &&
           curnotestate == PRESSED)  // ...unless note was just pressed
    curnotestate = STILLPRESSED;
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
    octavetxt = TextFormat("Octave %d+1", globaloctave + 4);
  else if (octaveoffset == -1)
    octavetxt = TextFormat("Octave %d-1", globaloctave + 4);
  else
    octavetxt = TextFormat("Octave %d", globaloctave + 4);

  voltxt = TextFormat("VOL %d%%", (int)(sustainvol * 100));
}

void draw_wave() {
  if (actualvol == 0) {
    Vector2 start = {0, screenheight / 2};
    Vector2 end = {screenwidth, screenheight / 2};
    // Shadow
    DrawLineEx((Vector2){0, screenheight / 2 + 3},
               (Vector2){screenwidth, screenheight / 2 + 3}, 3, BLACK);
    // Actual
    DrawLineEx(start, end, 3, WHITE);
    return;
  }

  float dphase = actualfreq / WAVEXSCALE;
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
    // Shadow
    DrawLineEx((Vector2){i + 3, y + 2}, (Vector2){i + 5, y_ + 2}, 2, BLACK);
    // Actual
    DrawLineEx(start, end, 2, WHITE);
    if (phase >= 1)
      phase = fmodf(phase, 1.0);
  }
}

void draw_wavetype_icon() {
  // WAVE TYPE
  bool hovering =
      dist(GetMouseX(), GetMouseY(), XMARGIN + 20, YMARGIN + 23) < 400;
  switch (wavetype) {
    case PULSE:
      if (hovering)
        DrawTextureEx(texture_pulsewaveglow,
                      (Vector2){XMARGIN - 9, YMARGIN - 1}, 0.0, 0.5, WHITE);
      else
        DrawTextureEx(texture_pulsewave, (Vector2){XMARGIN, YMARGIN + 5}, 0.0,
                      0.5, WHITE);
      break;
    case TRI:
      if (hovering)
        DrawTextureEx(texture_triwaveglow, (Vector2){XMARGIN - 7, YMARGIN + 1},
                      0.0, 0.5, WHITE);
      else
        DrawTextureEx(texture_triwave, (Vector2){XMARGIN, YMARGIN + 5}, 0.0,
                      0.5, WHITE);
      break;
    case SAW:
      if (hovering)
        DrawTextureEx(texture_sawwaveglow, (Vector2){XMARGIN - 7, YMARGIN + 1},
                      0.0, 0.5, WHITE);
      else
        DrawTextureEx(texture_sawwave, (Vector2){XMARGIN, YMARGIN + 5}, 0.0,
                      0.5, WHITE);
      break;
  }

  if (hovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    wavetype++;
    if (wavetype >= 3)
      wavetype = 0;
  }
}

void draw_bottom_icons() {
  bool hovering = dist(GetMouseX(), GetMouseY(), XMARGIN + 20,
                       screenheight - YMARGIN - 23) < 400;
  if (hovering)
    DrawTextureEx(texture_adsrglow,
                  (Vector2){XMARGIN - 7, screenheight - YMARGIN - 53}, 0.0, 0.5,
                  WHITE);
  else
    DrawTextureEx(texture_adsr, (Vector2){XMARGIN, screenheight - YMARGIN - 50},
                  0.0, 0.5, WHITE);

  if (hovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    show_adsr = !show_adsr;
    if (show_adsr)
      init_adsr_nodes();
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
  ClearBackground(BG);

  draw_wave();
  draw_wavetype_icon();
  draw_bottom_icons();

  // Draw octave and note text
  setdisplaytxt();
  // TODO: separate function for drawing shadows
  DrawText(octavetxt, XMARGIN + 73, YMARGIN + 3, FONTSIZE, BLACK);
  DrawText(octavetxt, XMARGIN + 70, YMARGIN, FONTSIZE, WHITE);
  if (PLAYING) {
    DrawText(notetxt, XMARGIN + 73, YMARGIN + 23, FONTSIZE * 2, BLACK);
    DrawText(notetxt, XMARGIN + 70, YMARGIN + 20, FONTSIZE * 2, WHITE);
  }

  // Draw volume level
  int x1 = XMARGIN + 190;
  int x2 = XMARGIN + 350;
  int y1 = YMARGIN + 10;
  int y2 = YMARGIN + 50;
  Vector2 v1 = {x1, y2};
  Vector2 v2 = {x2, y2};
  Vector2 v3 = {x2, y1};
  // sustainvol^0.7 is taken so that the gray fill is visible at low volumes
  int x3 = x1 + powf(sustainvol, 0.7) * (x2 - x1);
  int y3 = y1 + (1.0 - powf(sustainvol, 0.7)) * (y2 - y1);
  // Shadows
  Vector2 v1_ = {x1 + 3, y2 + 3};
  Vector2 v2_ = {x2 + 3, y2 + 3};
  Vector2 v3_ = {x2 + 3, y1 + 3};
  DrawLineEx(v1_, v2_, 3, BLACK);
  DrawLineEx((Vector2){x2 + 3, y2 + 4}, (Vector2){x2 + 3, y1 - 2}, 3, BLACK);
  DrawLineEx(v3_, v1_, 3, BLACK);
  // Actual
  DrawTriangle(v1, (Vector2){x3, y2}, (Vector2){x3, y3}, WHITE);
  DrawLineEx(v1, v2, 3, WHITE);
  DrawLineEx((Vector2){x2, y2 + 1}, (Vector2){x2, y1 - 1}, 3, WHITE);
  DrawLineEx(v3, v1, 3, WHITE);

  // if (issustain)
  //   DrawTextLL("SUSTAIN", XMARGIN, screenheight - YMARGIN - YSPACE, FONTSIZE,
  //              WHITE);
  // if (isglisslock)
  //   DrawTextLR("GLISS LOCK", screenwidth - XMARGIN, screenheight - YMARGIN,
  //              FONTSIZE, WHITE);
  // if (isconstvol)
  //   DrawTextLR("CONST VOL", screenwidth - XMARGIN,
  //              screenheight - YMARGIN - YSPACE, FONTSIZE, WHITE);
  // if (isrecording)
  //   DrawTextUR("RECORDING", screenwidth - XMARGIN, YMARGIN, FONTSIZE, WHITE);

  if (show_adsr)
    draw_adsr_gui();
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

  texture_pulsewave = LoadTexture("assets/pulsewave.png");
  texture_triwave = LoadTexture("assets/triwave.png");
  texture_sawwave = LoadTexture("assets/sawwave.png");
  texture_pulsewaveglow = LoadTexture("assets/pulsewave_glow.png");
  texture_triwaveglow = LoadTexture("assets/triwave_glow.png");
  texture_sawwaveglow = LoadTexture("assets/sawwave_glow.png");
  texture_adsr = LoadTexture("assets/adsr.png");
  texture_adsrglow = LoadTexture("assets/adsr_glow.png");

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
    if (IsCursorHidden())
      update_octave();
    update_notestates();
    if (frames_toautogliss == 0) {
      if (IsCursorHidden())
        update_pitchbend();
      update_effects();
      if (IsCursorHidden())
        update_gliss();
      update_vib();
    }
    update_actualfreq();
    if (IsCursorHidden())
      update_sustainvol();
    update_actualvol();

    if (IsKeyPressed(KEY_RIGHT_SHIFT)) {
      if (cursorenabled)
        DisableCursor();
      else
        EnableCursor();
      cursorenabled = !cursorenabled;
    }

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
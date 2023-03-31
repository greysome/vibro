/***
Version of my-kb meant for joystick.
Personally, I found it harder to use than a mouse.
***/

#include <string.h>
#include <stdio.h>
#include <math.h>
#include "tinywav/tinywav.h"
#include "raylib.h"

/** C macros **/
#define clamp(x,min,max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define abs(x) ((x) < 0 ? -(x) : (x))
#define mod(a,b) (((a) % (b) < 0 ? (a) % (b) + (b) : (a) % (b)))
// C's fmodf behaves more like a remainder
#define fmodf2(a,b) (fmodf((a),(b)) < 0 ? fmodf((a),(b)) + (b) : fmodf((a),(b)))

/** Raylib macros **/
#define keydown(key) (IsKeyDown(KEY_##key))
#define keypressed(key) (IsKeyPressed(KEY_##key))
#define mousedown(btn) (IsMouseButtonDown(MOUSE_BUTTON_##btn))
#define joybtndown(btn) (IsGamepadButtonDown(0,btn))
#define joyaxis(axis) (GetGamepadAxisMovement(0,axis))

/** Global settings **/
#define NORMALWIDTH 1400
#define NORMALHEIGHT 900
#define WIDTH GetScreenWidth()
#define HEIGHT GetScreenHeight()
#define FPS 60
#define MAXSAMPLES_PER_UPDATE 9192
#define SAMPLERATE 96000
#define BITDEPTH 16
#define RECORDFILE "out.wav"
#define MINOCTAVE -4
#define MAXOCTAVE 2
// How loud should this program be compared to the actual
// system volume, from 0 to 1?
#define MAXVOL 0.3
// Number of seconds for the drawn wave to move one full cycle
#define WAVESPEED 2.0
// Controls the scale of the horizontal axis for the drawn wave.
// The lower, the more bunched up the wave will be
#define WAVEXSCALE 20000
// Scale of vertical axis.
#define WAVEYSCALE 100

// Moving up a semitone increases the Hz by this factor
#define SEMITONE 1.05946
// In Hz
#define C4 261.6
#define CURFREQ (C4 * \
                 pow(2, octave) * \
                 pow(SEMITONE, curnote) * \
                 freqoffset * \
                 freqoffset2 * \
                 freqoffset3 * \
                 freqoffset_drop)

#define TRI 0
#define SAW 1
#define PULSE 2
int wavetype = SAW;

/** Note/frequency **/
// The number of semitones above the C at the current octave.
// Only changes when a new note is pressed.
int curnote;
// Fine-control of frequency via the mousewheel. For pitch bends.
// Note freqoffset* is _multiplied_ in to get the final frequency.
float freqoffset = 1;
// Changing frequency via mouse movement. For glisses.
float freqoffset2 = 1;
// Changing frequency via space bar. For vibrato.
float freqoffset3 = 1;

// There are two ways to change the octave, via the up and down
// arrow keys and via left and right mouse clicks.
// The former method changes preoctave, whereas the latter
// changes octaveoffset. Thus the final octave is computed as
// preoctave + octaveoffset.
// C4 is considered to be octave 0.
int preoctave = 0;
int octaveoffset = 0;
int octave;

// How far are we in the current cycle of the wave from 0 to 1?
float curphase = 0.0;

/** Volume **/
// Volume of note when held, ignoring attacks and releases,
// as a proportion of MAXVOL.
// Can be controlled by the number keys on the keyboard.
float notevol = 1.0;
// Similar by notevol, but updates frame-by-frame according
// to attacks and releases
float curvol = 1.0;

/** Volume envelopes **/
// A volume envelope is hardcoded to have exactly 60 frames,
// which is also the set FPS. Each entry represents a proportion
// of notevol.
#define envel6(a,b,c,d,e,f) \
  { \
    a,a,a,a,a,a,a,a,a,a, \
    b,b,b,b,b,b,b,b,b,b, \
    c,c,c,c,c,c,c,c,c,c, \
    d,d,d,d,d,d,d,d,d,d, \
    e,e,e,e,e,e,e,e,e,e, \
    f,f,f,f,f,f,f,f,f,f, \
  }

#define envel6_rapid_in(a,b,c,d,e,f) \
  { \
    a,a,a,b,b,b,c,c,c,d, \
    d,d,e,e,e,f,f,f,1,1, \
    1,1,1,1,1,1,1,1,1,1, \
    1,1,1,1,1,1,1,1,1,1, \
    1,1,1,1,1,1,1,1,1,1, \
    1,1,1,1,1,1,1,1,1,1, \
  }

#define envel6_rapid_out(a,b,c,d,e,f) \
  { \
    a,a,a,b,b,b,c,c,c,d, \
    d,d,e,e,e,f,f,f,0,0, \
    0,0,0,0,0,0,0,0,0,0, \
    0,0,0,0,0,0,0,0,0,0, \
    0,0,0,0,0,0,0,0,0,0, \
    0,0,0,0,0,0,0,0,0,0, \
  }

#define NOENVEL_IN envel6_rapid_in(1,1,1,1,1,1)
#define NOENVEL_OUT envel6_rapid_out(0,0,0,0,0,0)

float attackenvel[FPS] = envel6_rapid_in(1.5,1.2,1.1,1.0,1.0,1.0);
float releaseenvel[FPS] = envel6_rapid_out(1,0.5,0.2,0.0,0.0,0.0);

/** Note press data */
#define RELEASED 0
#define STILLRELEASED 1
#define PRESSED 2
#define STILLPRESSED 3
int curstate = STILLRELEASED;
int prevstate;
#define PLAYING ((curstate == PRESSED) || (curstate == STILLPRESSED))
// Number of frames elapsed since the last note that was played
// (including silence).
int frames_newnote;
// Number of frames elasped since note released.
int frames_releasenote;

#define NOTETABLE_SIZE 17
// Keeps track of which notes are pressed
// Used to determine which note to play out, in the case
// that multiple keys were pressed.
short notetable[NOTETABLE_SIZE];
short notetable_prev[NOTETABLE_SIZE];

/** Vibrato **/
int prevvibstate;
int curvibstate = STILLRELEASED;
int frames_onspace, frames_betweenspace;
float vibspeed = 0, vibdepth = 1;
float vibphase = 0;
#define PREVVIBPLAYING ((prevvibstate == PRESSED) || (prevvibstate == STILLPRESSED))
#define VIBPLAYING ((curvibstate == PRESSED) || (curvibstate == STILLPRESSED))

/** Effects **/
#define MAXDROPFRAMES (FPS/3)
int frames_drop = 0;
float freqoffset_drop = 1; // The 2XX effect in Famitracker

/** Controls **/
#define DROP_PRESSED (keypressed(LEFT_ALT) || keypressed(RIGHT_ALT))
#define NOTEVOL_AMT (0.5 + 0.5 * joyaxis(0))
#define FREQOFFSET1_AMT (joybtndown(11) ? 1 : pow(SEMITONE, -3*joyaxis(1)))
//#define FREQOFFSET2_AMT (joybtndown(11) ? pow(SEMITONE, -12*joyaxis(1)) : 1)
#define FREQOFFSET2_AMT (joybtndown(11) ? 1 : pow(SEMITONE, 12*joyaxis(2)))
#define TEMPDOWNOCTAVE_ON joybtndown(15)
#define TEMPUPOCTAVE_ON joybtndown(17)

/** Text displays **/
const char *notetxt;
const char *octavetxt;
const char *voltxt;

/** Recording-related **/
// Are we recording?
int recording = 0;
// Struct to facilitate .wav output
TinyWav tw;

/** Synthesis functions. Outputs a value between -1 and 1. **/
float tri(float phase) {
  return 4*abs(phase-0.5) - 1;
}

float nes_tri(float phase) {  
  float y = tri(phase);
  return y - fmodf2(y, 2.0/16);
}

float nes_saw(float phase) {
  float y = 2 * (phase-0.5);
  return y - fmodf2(y, 2.0/16);
}

float nes_pulse(float phase) {
  return phase >= 0.5 ? 1 : -1;
}
  
void synthesise(void *buffer, unsigned int frames) {
  short *d = (short *) buffer;
  float wavbuf[frames];
	float amplitude;

  for (unsigned int i = 0; i < frames; i++) {
    // The actual synthesis
    if (wavetype == TRI)
      amplitude = nes_tri(curphase);
    else if (wavetype == SAW)
      amplitude = nes_saw(curphase);
    else if (wavetype == PULSE)
      amplitude = nes_pulse(curphase);
    // Scale it to match the actual amplitude for the output format
    amplitude *= curvol * MAXVOL;
    d[i] = (short) (amplitude * pow(2, BITDEPTH));
    if (recording) {
      // For some reason, without this line the .wav is completely
      // corrupted.
			amplitude /= MAXVOL;
      wavbuf[i] = amplitude;
    }

    curphase += CURFREQ / SAMPLERATE;
    if (curphase >= 1)
      curphase = fmodf(curphase, 1.0);
  }
	tinywav_write_f(&tw, wavbuf, frames);
}

/**
  Functions to update the sound characteristics based on user
  input.
**/
void update_notevol() {
  notevol = NOTEVOL_AMT;
}

void update_curvol() {
  if (PLAYING)
    curvol = (frames_newnote <= FPS-1) ? \
      notevol * attackenvel[frames_newnote] : \
      notevol;
  else
    curvol = (frames_releasenote <= FPS-1) ? \
      notevol * releaseenvel[frames_releasenote] : \
      0;
}

void update_pitchbend() {
  if (curstate == PRESSED)
    freqoffset = 1;
  freqoffset = FREQOFFSET1_AMT;
}

void update_gliss() {
  if (curstate == PRESSED)
    freqoffset2 = 1;
  freqoffset2 = FREQOFFSET2_AMT;
}

void update_vib() {
  vibphase += vibspeed / FPS;

  prevvibstate = curvibstate;
  if (keydown(SPACE))
    curvibstate = VIBPLAYING ? STILLPRESSED : PRESSED;
  else
    curvibstate = VIBPLAYING ? RELEASED : STILLRELEASED;

  if (!PREVVIBPLAYING && VIBPLAYING) {
    vibspeed = 1.0 / frames_betweenspace;
    frames_betweenspace = 0;
  }
  else {
    if (frames_betweenspace > 30) {
      // Make vibrato decay if not 'replenished'
      vibspeed *= 0.6;
      vibphase *= 0.6;
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
  if (vibphase >= 1) vibphase = 0;
  freqoffset3 = pow(vibdepth, sinf(vibphase*2*PI));
}

void update_wavetype() {
  if (keypressed(MINUS))
    wavetype = TRI;
  if (keypressed(EQUAL))
    wavetype = SAW;
  if (keypressed(BACKSPACE))
    wavetype = PULSE;
}

void update_octave() {
  if (keypressed(DOWN))
    preoctave = clamp(preoctave-1, MINOCTAVE, MAXOCTAVE);
  if (keypressed(UP))
    preoctave = clamp(preoctave+1, MINOCTAVE, MAXOCTAVE);

  if (TEMPDOWNOCTAVE_ON && preoctave > MINOCTAVE)
    octaveoffset = -1;
  else if (TEMPUPOCTAVE_ON && preoctave < MAXOCTAVE)
    octaveoffset = 1;
  if (!TEMPDOWNOCTAVE_ON && !TEMPUPOCTAVE_ON)
    octaveoffset = 0;

  octave = preoctave + octaveoffset;
}

void update_notetables() {
  for (int i = 0; i < NOTETABLE_SIZE; i++)
    notetable_prev[i] = notetable[i];

#define key2note(key,note) notetable[note] = keydown(key)
  key2note(Z,0);
  key2note(S,1);
  key2note(X,2);
  key2note(D,3);
  key2note(C,4);
  key2note(V,5);
  key2note(G,6);
  key2note(B,7);
  key2note(H,8);
  key2note(N,9);
  key2note(J,10);
  key2note(M,11);
  key2note(COMMA,12);
  key2note(L,13);
  key2note(PERIOD,14);
  key2note(SEMICOLON,15);
  key2note(SLASH,16);
#undef key2note
}

void update_notestates() {
  prevstate = curstate;
  // Any change to the notetable?
  int changed = 0;
  // A note that has remained pressed, to use as current note
  // in case some other note is released.
  // Otherwise, x = -1 indicates a note release.
  int x = -1;
  for (int i = 0; i < NOTETABLE_SIZE; i++) {
    if (notetable_prev[i] == 1 && notetable[i] == 1)
      x = i;
    // If a new note has been pressed, use that
    else if (notetable_prev[i] == 0 && notetable[i] == 1) {
      changed = 1;
      x = i;
      break;
    }
    else if (notetable_prev[i] == 1 && notetable[i] == 0)
      changed = 1;
  }

  if (changed) {
    if (x == -1)
      curstate = RELEASED;
    else {
      curstate = PRESSED;
      curnote = x;
    }
  }
  else
    curstate = PLAYING ? STILLPRESSED : STILLRELEASED;

  // Update frames
  if (curstate != STILLPRESSED)
    frames_newnote = 0;
  else
    frames_newnote++;

  if (curstate != STILLRELEASED)
    frames_releasenote = 0;
  else
    frames_releasenote++;
}

void update_effects() {
  if (DROP_PRESSED)
    frames_drop = 1;

  if (frames_drop > 0) {
    if (++frames_drop <= MAXDROPFRAMES)
      if (curstate == PRESSED) {
        frames_drop = 0;
        freqoffset_drop = 1;
      }
      else
        freqoffset_drop *= 0.9;
    else {
      curstate = RELEASED;
      frames_drop = 0;
      freqoffset_drop = 1;
    }
  }
}

/** Drawing functions. **/
void setdisplaytxt() {
  int abs_note = curnote + octave * 12;
  int scaledegree = mod(abs_note, 12);

#define scaledeg2txt(x,txt) \
  if (scaledegree == x) { \
    notetxt = TextFormat(txt"%d", octave + (curnote/12) + 4);  \
  } \

  scaledeg2txt(0,"C-");
  scaledeg2txt(1,"C#");
  scaledeg2txt(2,"D-");
  scaledeg2txt(3,"D#");
  scaledeg2txt(4,"E-");
  scaledeg2txt(5,"F-");
  scaledeg2txt(6,"F#");
  scaledeg2txt(7,"G-");
  scaledeg2txt(8,"G#");
  scaledeg2txt(9,"A-");
  scaledeg2txt(10,"A#");
  scaledeg2txt(11,"B-");
#undef scaledeg2txt

  if (octaveoffset == 1)
    octavetxt = TextFormat("OCTAVE %d+1", preoctave+4);
  else if (octaveoffset == -1)
    octavetxt = TextFormat("OCTAVE %d-1", preoctave+4);
  else
    octavetxt = TextFormat("OCTAVE %d", preoctave+4);

  voltxt = TextFormat("VOL %d%%", (int) (notevol*100));
}

void drawwave() {
  if (curvol == 0) {
    Vector2 start = {0, HEIGHT/2};
    Vector2 end = {WIDTH, HEIGHT/2};
    DrawLineEx(start, end, 2, WHITE);
    return;
  }

  float dphase = CURFREQ / WAVEXSCALE;
  float phase = fmodf2((float) GetTime(), WAVESPEED) / WAVESPEED;
  int y, y_;
  for (int i = 0; i < WIDTH; i += 2) {
    phase += dphase;
    float nextphase = phase+dphase;
    if (nextphase >= 1)
      nextphase -= 1;
    if (wavetype == TRI) {
      y = HEIGHT/2 + 100 * curvol * nes_tri(phase);
      y_ = HEIGHT/2 + 100 * curvol * nes_tri(nextphase);
    }
    else if (wavetype == SAW) {
      y = HEIGHT/2 + 100 * curvol * nes_saw(phase);
      y_ = HEIGHT/2 + 100 * curvol * nes_saw(nextphase);
    }
    else if (wavetype == PULSE) {
      y = HEIGHT/2 + 100 * curvol * nes_pulse(phase);
      y_ = HEIGHT/2 + 100 * curvol * nes_pulse(nextphase);
    }
    Vector2 start = {i, y};
    Vector2 end = {i+2, y_};
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
#define DrawTextLL(text,x,_y,fontsize,color) \
  DrawText(text, x, _y - MeasureTextEx(DEFAULTFONT,text,fontsize,0).y, \
     fontsize, color)
#define DrawTextLR(text,x,_y,fontsize,color) \
  DrawText(text, x - MeasureText(text,fontsize), \
           _y - MeasureTextEx(DEFAULTFONT,text,fontsize,0).y, \
           fontsize, color)
#define DrawTextUR(text,x,y,fontsize,color) \
  DrawText(text, x - MeasureText(text,fontsize), y, fontsize, color)

void draw() {
  ClearBackground(DARKGRAY);
  if (wavetype == TRI)
    DrawText("NES TRI", XMARGIN, YMARGIN, FONTSIZE, WHITE);
  else if (wavetype == SAW)
    DrawText("NES SAW", XMARGIN, YMARGIN, FONTSIZE, WHITE);
  else if (wavetype == PULSE)
    DrawText("PULSE", XMARGIN, YMARGIN, FONTSIZE, WHITE);

  setdisplaytxt();
  DrawText(octavetxt, XMARGIN, YMARGIN+YSPACE,
           FONTSIZE, WHITE);
  if (PLAYING)
    DrawText(notetxt, XMARGIN, YMARGIN+2*YSPACE,
             FONTSIZE, WHITE);

  DrawTextLL(voltxt, XMARGIN, HEIGHT-YMARGIN, FONTSIZE, WHITE);

  if (recording)
    DrawTextUR("RECORDING", WIDTH-XMARGIN, YMARGIN,
               FONTSIZE, WHITE);


  DrawTextLR(TextFormat("Button %d", GetGamepadButtonPressed()),
             WIDTH-XMARGIN,
             HEIGHT-YMARGIN,
             FONTSIZE,
             WHITE);
  for (int i = 0; i < GetGamepadAxisCount(0); i++)
    DrawTextLR(TextFormat("Axis %d: %f", i, GetGamepadAxisMovement(0,i)),
               WIDTH-XMARGIN,
               HEIGHT-(i+2)*YMARGIN,
               FONTSIZE,
               WHITE);

  drawwave();
}


int main() {
  InitWindow(NORMALWIDTH, NORMALHEIGHT, "my keyboard");
  InitAudioDevice();
  SetAudioStreamBufferSizeDefault(MAXSAMPLES_PER_UPDATE);

  AudioStream stream = LoadAudioStream(SAMPLERATE, BITDEPTH, 1);
  SetAudioStreamCallback(stream, synthesise);
  PlayAudioStream(stream);

  DisableCursor();
  SetTargetFPS(FPS);
  SetGamepadMappings("030000006d04000015c2000000000000,Logitech Extreme 3D,platform:Windows,a:b10,b:b11,x:b8,y:b9,back:b6,guide:b7,start:b2,leftstick:b3,rightstick:b4,leftshoulder:b5,rightshoulder:b0,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,lefttrigger:a3~,");
  while (!WindowShouldClose()) {
    update_wavetype();
    update_notetables();
    update_notestates();
    update_octave();
    update_gliss();
    update_pitchbend();
    update_effects();
    update_vib();
    update_notevol();
    update_curvol();

    if (keypressed(GRAVE)) {
      if (!recording)
        tinywav_open_write(&tw,
          1, // number of channels
          SAMPLERATE,
          TW_FLOAT32,
          TW_INLINE,
          RECORDFILE
        );
      else
        tinywav_close_write(&tw);
      recording = !recording;
    }

    if (keypressed(TAB)) {
      if (IsWindowFullscreen()) {
        ToggleFullscreen();
        SetWindowSize(NORMALWIDTH, NORMALHEIGHT);
      }
      else {
        SetWindowSize(GetMonitorWidth(0), GetMonitorHeight(0));
        ToggleFullscreen();
      }
    }

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
#ifndef _GUI
#define _GUI

#include "raylib.h"

#define XMARGIN 40
#define YMARGIN 20
// Window dimensions upon program startup, and also after exiting fullscreen mode.
// May not be equal to *current* window dimensions (which is instead stored in
// screen_width/screen_height), as the user can freely resize the window.
#define INITIAL_WIDTH 1200
#define INITIAL_HEIGHT 700

// See comment above INITIAL_WIDTH/INITIAL_HEIGHT
extern int screen_width;
extern int screen_height;

#define SHIFT_DOWN (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT))

/** I follow Raylib's CamelCase convention for the following functions. **/
void DrawShadowedText(const char *text, int x, int y, int font_size, Color color);
void DrawShadowedTextCenter(const char *text, int x, int y, int font_size, Color color);
void DrawShadowedTextNE(const char *text, int x, int y, int font_size, Color color);
void DrawShadowedTextSE(const char *text, int x, int y, int font_size, Color color);
int DrawAndMeasureShadowedText(const char *text, int x, int y, int font_size, Color color);

#endif
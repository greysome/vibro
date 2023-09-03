#ifndef SUBGUI
#define SUBGUI

#include "debug.h"
#include "raylib.h"
#include "globals.h"
#include "util.h"

typedef struct {
  float x;
  float y;
} RelativeCoords;

typedef struct {
  int x;
  int y;
  int w;
  int h;
  int m;  // Margins
} SubGui;

RelativeCoords absolute_to_relative(Vector2 v, SubGui g) {
  float x = (v.x - (g.x + g.m)) / (g.w - 2 * g.m);
  float y = (v.y - (g.y + g.m)) / (g.h - 2 * g.m);
  x = clamp(x, 0, 1);
  y = clamp(y, 0, 1);
  return (RelativeCoords){x, y};
}

Vector2 relative_to_absolute(RelativeCoords c, SubGui g) {
  return (Vector2){g.x + g.m + c.x * (g.w - 2 * g.m),
                   g.y + g.m + c.y * (g.h - 2 * g.m)};
}

bool within_subgui(Vector2 v, SubGui g) {
  return g.x + g.m <= v.x && v.x <= g.x + g.w - g.m && g.y + g.m <= v.y &&
         v.y <= g.y + g.h - g.m;
}

void draw_subgui_border(SubGui g) {
  DrawRectangle(g.x + 5, g.y + 5, g.w, g.h, SHADOW);
  DrawRectangle(g.x, g.y, g.w, g.h, SUBGUI_BG);
  DrawRectangleLines(g.x, g.y, g.w, g.h, WHITE);
}

#endif
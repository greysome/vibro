/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef ADSR
#define ADSR

#include "debug.h"
#include "globals.h"
#include "raylib.h"
#include "util.h"

#define ADSR_X 20
#define ADSR_BG     \
  (Color) {         \
    60, 75, 65, 255 \
  }
#define ADSR_Y (screenheight - 250)
#define ADSR_W 500
#define ADSR_H 150
#define ADSR_M 30            // Margin
#define ADSR_NODESIZE 10     // Size of node
#define ADSR_FRAMESTEP 0.02  // How long is one frame from 0 to 1?

typedef struct {
  float x;
  float y;
  bool pressed;
} ADSRNode;

ADSRNode node_start;
ADSRNode node_attack;
ADSRNode node_decay;
ADSRNode node_sustain;
ADSRNode node_release;
bool show_adsr = false;
int adsr_hovered = 0;
int adsr_selected = 0;

float to_attackframes() {
  return node_attack.x / ADSR_FRAMESTEP;
}

float to_attackpeak() {
  return 1.0 / (1.0 - node_decay.y);
}

float to_decayframes() {
  return (node_decay.x - node_attack.x) / ADSR_FRAMESTEP;
}

float to_sustaindecayframes() {
  // TODO: document properly
  int displayframes = (node_sustain.x - node_decay.x) / ADSR_FRAMESTEP;
  float displaydecayfactor =
      (node_sustain.y - node_decay.y) / (1.0 - node_decay.y);
  return displayframes / displaydecayfactor;
}

float to_releasedecayframes() {
  return (1.0 - node_sustain.x) / ADSR_FRAMESTEP;
}

float node_decay_x() {
  return ADSR_FRAMESTEP * (attackframes + decayframes);
}

float node_decay_y() {
  return 1.0 - 1.0 / attackpeak;
}

float node_sustain_x() {
  return 1.0 - ADSR_FRAMESTEP * releasedecayframes;
}

float node_sustain_y() {
  float dx = node_decay_x();
  float dy = node_decay_y();
  float sx = node_sustain_x();
  return 1.0 - ((1.0 - dy) -
                (sx - dx) * (1.0 - dy) / sustaindecayframes / ADSR_FRAMESTEP);
}

void init_adsr_nodes() {
  node_start = (ADSRNode){0.0, 1.0, false};
  node_attack = (ADSRNode){ADSR_FRAMESTEP * attackframes, 0.0, false};
  node_decay = (ADSRNode){node_decay_x(), node_decay_y(), false};
  node_sustain = (ADSRNode){node_sustain_x(), node_sustain_y(), false};
  node_release = (ADSRNode){1.0, 1.0, false};
  node_attack.x = clamp(node_attack.x, 0, node_decay.x - ADSR_FRAMESTEP);
  node_decay.x = clamp(node_decay.x, node_attack.x + ADSR_FRAMESTEP,
                       node_sustain.x - ADSR_FRAMESTEP);
  node_sustain.x = clamp(node_sustain.x, node_decay.x + ADSR_FRAMESTEP,
                         1.0 - ADSR_FRAMESTEP);
}

ADSRNode coords_to_node(Vector2 v) {
  float x = (v.x - (ADSR_X + ADSR_M)) / ADSR_W;
  float y = (v.y - (ADSR_Y + ADSR_H)) / ADSR_H;
  x = clamp(x, 0, 1);
  y = clamp(y, 0, 1);
  return (ADSRNode){x, y, false};
}

Vector2 node_to_coords(ADSRNode n) {
  return (Vector2){ADSR_X + ADSR_M + n.x * (ADSR_W - 2 * ADSR_M),
                   ADSR_Y + ADSR_M + n.y * (ADSR_H - 2 * ADSR_M)};
}

bool hovering_on_node(ADSRNode n) {
  int x = GetMouseX();
  int y = GetMouseY();
  Vector2 v = node_to_coords(n);
  return (x - v.x) * (x - v.x) + (y - v.y) * (y - v.y) <
         ADSR_NODESIZE * ADSR_NODESIZE;
}

void draw_node(ADSRNode n, bool hovered) {
  Vector2 v = node_to_coords(n);
  DrawRectangle(v.x - ADSR_NODESIZE / 2 + 3, v.y - ADSR_NODESIZE / 2 + 3,
                ADSR_NODESIZE, ADSR_NODESIZE, SHADOW);
  DrawRectangle(v.x - ADSR_NODESIZE / 2, v.y - ADSR_NODESIZE / 2, ADSR_NODESIZE,
                ADSR_NODESIZE, hovered ? WHITE : ADSR_BG);
  DrawRectangleLines(v.x - ADSR_NODESIZE / 2, v.y - ADSR_NODESIZE / 2,
                     ADSR_NODESIZE, ADSR_NODESIZE, WHITE);
}

void draw_adsr_gui() {
  DrawRectangle(ADSR_X + 5, ADSR_Y + 5, ADSR_W, ADSR_H, SHADOW);
  DrawRectangle(ADSR_X, ADSR_Y, ADSR_W, ADSR_H, ADSR_BG);
  DrawRectangleLines(ADSR_X, ADSR_Y, ADSR_W, ADSR_H, WHITE);

  // TODO: Draw shadows as well
  DrawLineV(node_to_coords(node_start), node_to_coords(node_attack), WHITE);
  DrawLineV(node_to_coords(node_attack), node_to_coords(node_decay), WHITE);
  DrawLineV(node_to_coords(node_decay), node_to_coords(node_sustain), WHITE);
  DrawLineV(node_to_coords(node_sustain), node_to_coords(node_release), WHITE);

  if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    if (hovering_on_node(node_attack))
      adsr_selected = 1;
    else if (hovering_on_node(node_decay))
      adsr_selected = 2;
    else if (hovering_on_node(node_sustain))
      adsr_selected = 3;
    else
      adsr_selected = 0;
  }

  if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    adsr_selected = 0;

  ADSRNode n = coords_to_node(GetMousePosition());
  if (adsr_selected == 1) {
    node_attack.x = clamp(n.x, ADSR_FRAMESTEP, node_decay.x - ADSR_FRAMESTEP);
    node_decay.y = clamp(node_decay.y, node_attack.y + 0.05, 0.75);
    node_sustain.y = clamp(node_sustain.y, node_decay.y, 1.0);
  } else if (adsr_selected == 2) {
    node_decay.x = clamp(n.x, node_attack.x + ADSR_FRAMESTEP,
                         node_sustain.x - ADSR_FRAMESTEP);
    node_decay.y = clamp(n.y, node_attack.y + 0.05, 0.75);
    node_sustain.y = clamp(node_sustain.y, node_decay.y, 1.0);
  } else if (adsr_selected == 3) {
    node_sustain.x = clamp(n.x, node_decay.x + ADSR_FRAMESTEP,
                           node_release.x - ADSR_FRAMESTEP);
    node_sustain.y = clamp(n.y, node_decay.y, 1.0);
  }

  attackframes = to_attackframes();
  attackpeak = to_attackpeak();
  decayframes = to_decayframes();
  sustaindecayframes = to_sustaindecayframes();
  releasedecayframes = to_releasedecayframes();

  draw_node(node_start, false);
  draw_node(node_attack, (hovering_on_node(node_attack) && !adsr_selected) ||
                             adsr_selected == 1);
  draw_node(node_decay, (hovering_on_node(node_decay) && !adsr_selected) ||
                            adsr_selected == 2);
  draw_node(node_sustain, (hovering_on_node(node_sustain) && !adsr_selected) ||
                              adsr_selected == 3);
  draw_node(node_release, false);
}

#endif
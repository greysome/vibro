/**
 * Copyright (c) 2023-2023 Way Yan Win
 * This code is under the MIT License.
 */
#ifndef ADSR
#define ADSR

#include "debug.h"
#include "globals.h"
#include "raylib.h"
#include "subgui.h"
#include "util.h"

SubGui subgui_adsr;

#define ADSR_NODESIZE 10     // Size of node
#define ADSR_FRAMESTEP 0.02  // How much width in the GUI represents one frame?

RelativeCoords node_start;
RelativeCoords node_attack;
RelativeCoords node_decay;
RelativeCoords node_sustain;
RelativeCoords node_release;
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

void init_adsr_subgui() {
  subgui_adsr =
      (SubGui){.x = 20, .y = screenheight - 250, .w = 500, .h = 150, .m = 30};
  node_start = (RelativeCoords){0.0, 1.0};
  node_attack = (RelativeCoords){ADSR_FRAMESTEP * attackframes, 0.0};
  node_decay = (RelativeCoords){node_decay_x(), node_decay_y()};
  node_sustain = (RelativeCoords){node_sustain_x(), node_sustain_y()};
  node_release = (RelativeCoords){1.0, 1.0};
  node_attack.x = clamp(node_attack.x, 0, node_decay.x - ADSR_FRAMESTEP);
  node_decay.x = clamp(node_decay.x, node_attack.x + ADSR_FRAMESTEP,
                       node_sustain.x - ADSR_FRAMESTEP);
  node_sustain.x = clamp(node_sustain.x, node_decay.x + ADSR_FRAMESTEP,
                         1.0 - ADSR_FRAMESTEP);
}

bool hovering_on_node(RelativeCoords c) {
  RelativeCoords d = absolute_to_relative(GetMousePosition(), subgui_adsr);
  return within_subgui(GetMousePosition(), subgui_adsr) &&
         square(c.x - d.x) + square(c.y - d.y) < square(0.05);
}

void draw_node(RelativeCoords c, bool hovered) {
  Vector2 v = relative_to_absolute(c, subgui_adsr);
  DrawRectangle(v.x - ADSR_NODESIZE / 2 + 3, v.y - ADSR_NODESIZE / 2 + 3,
                ADSR_NODESIZE, ADSR_NODESIZE, SHADOW);
  DrawRectangle(v.x - ADSR_NODESIZE / 2, v.y - ADSR_NODESIZE / 2, ADSR_NODESIZE,
                ADSR_NODESIZE, hovered ? WHITE : SUBGUI_BG);
  DrawRectangleLines(v.x - ADSR_NODESIZE / 2, v.y - ADSR_NODESIZE / 2,
                     ADSR_NODESIZE, ADSR_NODESIZE, WHITE);
}

void draw_adsr_subgui() {
  draw_subgui_border(subgui_adsr);

  // TODO: Draw shadows as well
  DrawLineV(relative_to_absolute(node_start, subgui_adsr),
            relative_to_absolute(node_attack, subgui_adsr), WHITE);
  DrawLineV(relative_to_absolute(node_attack, subgui_adsr),
            relative_to_absolute(node_decay, subgui_adsr), WHITE);
  DrawLineV(relative_to_absolute(node_decay, subgui_adsr),
            relative_to_absolute(node_sustain, subgui_adsr), WHITE);
  DrawLineV(relative_to_absolute(node_sustain, subgui_adsr),
            relative_to_absolute(node_release, subgui_adsr), WHITE);

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

  RelativeCoords c = absolute_to_relative(GetMousePosition(), subgui_adsr);
  if (adsr_selected == 1) {
    node_attack.x = clamp(c.x, ADSR_FRAMESTEP, node_decay.x - ADSR_FRAMESTEP);
    node_decay.y = clamp(node_decay.y, node_attack.y + 0.05, 0.75);
    node_sustain.y = clamp(node_sustain.y, node_decay.y, 1.0);
  } else if (adsr_selected == 2) {
    node_decay.x = clamp(c.x, node_attack.x + ADSR_FRAMESTEP,
                         node_sustain.x - ADSR_FRAMESTEP);
    node_decay.y = clamp(c.y, node_attack.y + 0.05, 0.75);
    node_sustain.y = clamp(node_sustain.y, node_decay.y, 1.0);
  } else if (adsr_selected == 3) {
    node_sustain.x = clamp(c.x, node_decay.x + ADSR_FRAMESTEP,
                           node_release.x - ADSR_FRAMESTEP);
    node_sustain.y = clamp(c.y, node_decay.y, 1.0);
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
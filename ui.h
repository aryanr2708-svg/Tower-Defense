#ifndef UI_H
#define UI_H

#include "types.h"

void      ui_draw(const Game *g);                // draws everything for the current state
Rectangle ui_map_card(int index);                // clickable card on the menu
int       ui_menu_hit(Vector2 mouse);            // which map card is under the mouse, or -1
Rectangle ui_build_button(int index);            // tower button on the build bar
int       ui_build_hit(Vector2 mouse);           // which build button is under the mouse, or -1

#endif

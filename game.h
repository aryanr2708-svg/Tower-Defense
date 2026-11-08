#ifndef GAME_H
#define GAME_H

#include "types.h"

void game_init(Game *g);                         // clear everything, go to the menu
void game_start(Game *g, int mapIndex);          // clear everything, load a map, go to PLACE_CORE
void game_update(Game *g, float dt);             // input + state machine + simulation

bool game_mouse_tile(int *col, int *row);        // tile under the mouse, false if outside the window
bool game_can_place_core(const Game *g, int col, int row);
void game_place_core(Game *g, int col, int row);   // also builds the enemy routes

#endif

#ifndef TOWER_H
#define TOWER_H

#include "types.h"

extern const TowerType g_towerTypes[TOWER_KIND_COUNT];

typedef enum {
    BUILD_OK,
    BUILD_BAD_TILE,    // off the map, rock/water, or already built on
    BUILD_CAP,         // too many towers already
    BUILD_NO_COINS
} BuildResult;

int         tower_count(const Game *g);                 // how many are active
BuildResult tower_build_check(const Game *g, TowerKind kind, int col, int row);
bool        tower_can_build(const Game *g, TowerKind kind, int col, int row);
void        tower_place(Game *g, TowerKind kind, int col, int row);   // also pays the cost
void        tower_remove(Game *g, Tower *t);            // the single exit point for any tower
void        tower_update(Game *g, Tower *t, float dt);

#endif

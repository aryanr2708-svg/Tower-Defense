#ifndef MAPS_H
#define MAPS_H

#include "types.h"

extern const MapDef g_maps[MAP_COUNT];

// true for rock/water tiles and anything off the map
bool map_is_blocked(const MapDef *m, int col, int row);

#endif

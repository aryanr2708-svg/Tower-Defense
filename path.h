#ifndef PATH_H
#define PATH_H

#include "types.h"

// Fills f with the walking cost (x10, diagonals cost 14) from every tile to the core tile.
// -1 means "enemies can't get from there to the core". Rock and water are never walked on.
void path_build(const MapDef *m, int coreCol, int coreRow, FlowField *f);

// true if some tile on a screen edge the map allows spawning from can reach the core
bool path_edge_reachable(const MapDef *m, const FlowField *f);

// true if at least one tile along that edge (0 top, 1 right, 2 bottom, 3 left) can reach the core
bool path_side_reachable(const FlowField *f, int side);

// the neighbouring tile to step to from (col,row) on the way to the core.
// returns false if (col,row) is the core tile itself or cannot reach it.
bool path_next(const MapDef *m, const FlowField *f, int col, int row, int *nextCol, int *nextRow);

#endif

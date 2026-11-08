#include <limits.h>
#include <string.h>
#include "path.h"
#include "maps.h"

#define INF (1 << 29)

// a diagonal step is only allowed if both side tiles are free (no cutting corners)
static bool diagonal_ok(const MapDef *m, int col, int row, int dx, int dy)
{
    if (dx == 0 || dy == 0) return true;
    return !map_is_blocked(m, col + dx, row) && !map_is_blocked(m, col, row + dy);
}

void path_build(const MapDef *m, int coreCol, int coreRow, FlowField *f)
{
    bool done[ROWS][COLS];
    memset(done, 0, sizeof(done));

    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            f->d[r][c] = INF;

    if (!map_is_blocked(m, coreCol, coreRow))
        f->d[coreRow][coreCol] = 0;

    // Dijkstra: the grid is tiny, so scanning for the cheapest open tile is fast enough
    for (int iter = 0; iter < ROWS * COLS; iter++)
    {
        int bc = -1, br = -1, best = INF;
        for (int r = 0; r < ROWS; r++)
            for (int c = 0; c < COLS; c++)
                if (!done[r][c] && f->d[r][c] < best)
                {
                    best = f->d[r][c];
                    bc = c;
                    br = r;
                }
        if (bc < 0) break;                       // nothing left that can be reached
        done[br][bc] = true;

        for (int dy = -1; dy <= 1; dy++)
            for (int dx = -1; dx <= 1; dx++)
            {
                if (dx == 0 && dy == 0) continue;
                int c = bc + dx, r = br + dy;
                if (map_is_blocked(m, c, r)) continue;
                if (!diagonal_ok(m, bc, br, dx, dy)) continue;

                int cost = best + ((dx != 0 && dy != 0) ? 14 : 10);
                if (cost < f->d[r][c]) f->d[r][c] = cost;
            }
    }

    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            if (f->d[r][c] >= INF) f->d[r][c] = -1;
}

bool path_side_reachable(const FlowField *f, int side)
{
    for (int k = 0; k < (side == 0 || side == 2 ? COLS : ROWS); k++)
    {
        int c = (side == 3) ? 0 : (side == 1) ? COLS - 1 : k;
        int r = (side == 0) ? 0 : (side == 2) ? ROWS - 1 : k;
        if (f->d[r][c] >= 0) return true;
    }
    return false;
}

bool path_edge_reachable(const MapDef *m, const FlowField *f)
{
    for (int side = 0; side < 4; side++)
        if ((m->spawnSides & (1 << side)) && path_side_reachable(f, side))
            return true;
    return false;
}

bool path_next(const MapDef *m, const FlowField *f, int col, int row, int *nextCol, int *nextRow)
{
    if (f->d[row][col] <= 0) return false;       // 0 = the core itself, -1 = cut off

    bool found    = false;
    int  bestCost = INT_MAX;

    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0) continue;
            int c = col + dx, r = row + dy;
            if (map_is_blocked(m, c, r)) continue;
            if (f->d[r][c] < 0) continue;
            if (!diagonal_ok(m, col, row, dx, dy)) continue;

            int cost = f->d[r][c] + ((dx != 0 && dy != 0) ? 14 : 10);
            if (cost < bestCost)
            {
                bestCost  = cost;
                *nextCol  = c;
                *nextRow  = r;
                found     = true;
            }
        }
    return found;
}

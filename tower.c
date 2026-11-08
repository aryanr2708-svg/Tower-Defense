#include "tower.h"
#include "maps.h"
#include "projectile.h"

// name, description, cost, hp, life, range, cooldown, damage, splash, projectile speed, color
const TowerType g_towerTypes[TOWER_KIND_COUNT] = {
    { "Archer", "Fast shooter",  30, 20, 25.0f, TILE * 2.5f, 0.8f, 3, 0.0f,       TILE * 8.0f,
      { 0, 82, 172, 255 } },
    { "Cannon", "Splash blast",  60, 30, 25.0f, TILE * 3.0f, 1.6f, 8, TILE * 1.0f, TILE * 5.0f,
      { 110, 70, 30, 255 } },
    { "Decoy",  "Absorbs hits",  20, 60, 25.0f, 0.0f,        0.0f, 0, 0.0f,       0.0f,
      { 120, 120, 100, 255 } },
};

int tower_count(const Game *g)
{
    int n = 0;
    for (int i = 0; i < MAX_TOWERS; i++)
        if (g->towers[i].active) n++;
    return n;
}

BuildResult tower_build_check(const Game *g, TowerKind kind, int col, int row)
{
    if (!g->core.placed) return BUILD_BAD_TILE;
    if (col < 0 || col >= COLS || row < 0 || row >= ROWS) return BUILD_BAD_TILE;
    if (map_is_blocked(g->map, col, row)) return BUILD_BAD_TILE;
    if (g->occupied[row][col]) return BUILD_BAD_TILE;
    if (tower_count(g) >= MAX_ACTIVE_TOWERS) return BUILD_CAP;
    if (g->coins < g_towerTypes[kind].cost) return BUILD_NO_COINS;
    return BUILD_OK;
}

bool tower_can_build(const Game *g, TowerKind kind, int col, int row)
{
    return tower_build_check(g, kind, col, row) == BUILD_OK;
}

void tower_place(Game *g, TowerKind kind, int col, int row)
{
    const TowerType *tt = &g_towerTypes[kind];

    for (int i = 0; i < MAX_TOWERS; i++)
    {
        Tower *t = &g->towers[i];
        if (t->active) continue;

        t->col    = col;
        t->row    = row;
        t->pos    = (Vector2){ col * TILE + TILE / 2.0f, row * TILE + TILE / 2.0f };
        t->kind   = kind;
        t->timer  = 0.0f;
        t->hp     = tt->hp;
        t->life   = tt->life;
        t->active = true;
        g->occupied[row][col] = true;
        g->coins -= tt->cost;
        return;
    }
}

void tower_remove(Game *g, Tower *t)
{
    t->active = false;
    g->occupied[t->row][t->col] = false;      // the tile can be built on again
}

// index of the closest enemy inside the tower's range, or -1
static int find_target(const Game *g, const Tower *t, float range)
{
    int   best      = -1;
    float bestDist2 = range * range;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        if (!g->enemies[i].active) continue;

        float dx = g->enemies[i].pos.x - t->pos.x;
        float dy = g->enemies[i].pos.y - t->pos.y;
        float d2 = dx * dx + dy * dy;

        if (d2 <= bestDist2)
        {
            bestDist2 = d2;
            best = i;
        }
    }
    return best;
}

void tower_update(Game *g, Tower *t, float dt)
{
    const TowerType *tt = &g_towerTypes[t->kind];

    t->life -= dt;                             // the tower's time limit ticks down
    if (t->life <= 0.0f)
    {
        tower_remove(g, t);                    // time is up: it vanishes
        return;
    }

    if (tt->damage <= 0) return;               // decoys never shoot

    t->timer += dt;
    if (t->timer < tt->cooldown) return;       // still reloading

    int target = find_target(g, t, tt->range);
    if (target >= 0)
    {
        projectile_shoot(g, t->pos, target, tt);
        t->timer = 0.0f;
    }
}

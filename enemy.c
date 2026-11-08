#include <math.h>
#include "enemy.h"
#include "tower.h"
#include "path.h"

// name, hp, speed, damage, range, cooldown, radius, color, unlockTime, weight, behavior, reward
const EnemyType g_enemyTypes[ENEMY_KIND_COUNT] = {
    { "Grunt",  10, TILE * 2.0f, 2, TILE * 1.8f, 1.0f, TILE / 3.0f,
      { 230, 41, 55, 255 },  0.0f, 5, BEHAVIOR_NEAREST, 5 },
    { "Runner",  6, TILE * 3.5f, 3, TILE * 1.2f, 0.8f, TILE / 4.0f,
      { 255, 161, 0, 255 }, 15.0f, 3, BEHAVIOR_CORE_ONLY, 7 },
    { "Tank",   40, TILE * 1.0f, 5, TILE * 2.0f, 1.5f, TILE * 0.45f,
      { 190, 33, 55, 255 }, 35.0f, 2, BEHAVIOR_TOWERS_FIRST, 20 },
};

// tile index for a pixel coordinate, kept inside the map
static int tile_of(float v, int count)
{
    int t = (int)(v / TILE);
    if (t < 0) t = 0;
    if (t > count - 1) t = count - 1;
    return t;
}

// picks a random screen edge that the map allows AND that can reach the core (0..3), or -1
int enemy_random_side(const Game *g)
{
    int allowed[4], n = 0;
    for (int s = 0; s < 4; s++)
        if ((g->map->spawnSides & (1 << s)) && path_side_reachable(&g->flow, s))
            allowed[n++] = s;
    if (n == 0) return -1;
    return allowed[GetRandomValue(0, n - 1)];
}

// a random point on the given screen edge
static Vector2 edge_point(int side)
{
    switch (side)
    {
        case 0:  return (Vector2){ (float)GetRandomValue(0, SCREEN_W), 0.0f };
        case 1:  return (Vector2){ (float)SCREEN_W, (float)GetRandomValue(0, SCREEN_H) };
        case 2:  return (Vector2){ (float)GetRandomValue(0, SCREEN_W), (float)SCREEN_H };
        default: return (Vector2){ 0.0f, (float)GetRandomValue(0, SCREEN_H) };
    }
}

void enemy_spawn(Game *g, EnemyKind kind, int side)
{
    if (side < 0) side = enemy_random_side(g);
    if (side < 0) return;                        // no edge can reach the core

    // pick a spot on that edge that is not rock or water and can reach the core
    Vector2 start = { 0.0f, 0.0f };
    bool    found = false;
    for (int tries = 0; tries < 30 && !found; tries++)
    {
        start = edge_point(side);
        if (g->flow.d[tile_of(start.y, ROWS)][tile_of(start.x, COLS)] >= 0) found = true;
    }
    if (!found) return;

    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        Enemy *e = &g->enemies[i];
        if (e->active) continue;                 // find the first free slot

        e->pos      = start;
        e->kind     = kind;
        e->hp       = g_enemyTypes[kind].hp;
        e->atkTimer = 0.0f;
        e->zap      = 0.0f;
        e->zapTo    = e->pos;
        e->id       = g->nextEnemyId++;
        e->active   = true;
        return;
    }
    // pool full: the spawn is skipped
}

static float dist2(Vector2 a, Vector2 b)
{
    float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// decides what this enemy attacks right now: a tower index, TARGET_CORE or TARGET_NONE
static int find_building_target(const Game *g, const Enemy *e)
{
    const EnemyType *t = &g_enemyTypes[e->kind];
    float r2 = t->range * t->range;

    int   bestTower = TARGET_NONE;
    float bestD2    = r2;

    if (t->behavior != BEHAVIOR_CORE_ONLY)
    {
        for (int i = 0; i < MAX_TOWERS; i++)
        {
            if (!g->towers[i].active) continue;
            float d2 = dist2(g->towers[i].pos, e->pos);
            if (d2 <= bestD2)
            {
                bestD2    = d2;
                bestTower = i;
            }
        }
    }

    float coreD2     = dist2(g->core.pos, e->pos);
    bool  coreInRange = coreD2 <= r2;

    switch (t->behavior)
    {
        case BEHAVIOR_CORE_ONLY:
            return coreInRange ? TARGET_CORE : TARGET_NONE;

        case BEHAVIOR_TOWERS_FIRST:
            if (bestTower >= 0) return bestTower;
            return coreInRange ? TARGET_CORE : TARGET_NONE;

        default:   // BEHAVIOR_NEAREST
            if (bestTower >= 0 && (!coreInRange || bestD2 <= coreD2)) return bestTower;
            return coreInRange ? TARGET_CORE : TARGET_NONE;
    }
}

void enemy_update(Game *g, Enemy *e, float dt)
{
    const EnemyType *t = &g_enemyTypes[e->kind];

    if (e->zap > 0.0f) e->zap -= dt;                 // fade the attack line

    e->atkTimer += dt;
    if (e->atkTimer > t->cooldown) e->atkTimer = t->cooldown;

    int target = find_building_target(g, e);

    if (target != TARGET_NONE)                       // attack mode: stand still and hit
    {
        if (e->atkTimer >= t->cooldown)
        {
            e->atkTimer = 0.0f;
            e->zap      = 0.12f;

            if (target == TARGET_CORE)
            {
                g->core.hp -= t->damage;
                e->zapTo = g->core.pos;
            }
            else
            {
                Tower *tw = &g->towers[target];
                tw->hp -= t->damage;
                e->zapTo = tw->pos;
                if (tw->hp <= 0) tower_remove(g, tw);   // tower destroyed
            }
        }
        return;
    }

    // move mode: follow the shortest route around rock and water, one tile at a time
    Vector2 goal = g->core.pos;                  // fallback: straight at the core
    int col = tile_of(e->pos.x, COLS);
    int row = tile_of(e->pos.y, ROWS);
    int nc, nr;
    if ((col != g->core.col || row != g->core.row) &&
        path_next(g->map, &g->flow, col, row, &nc, &nr))
    {
        goal = (Vector2){ nc * TILE + TILE / 2.0f, nr * TILE + TILE / 2.0f };
    }

    float dx   = goal.x - e->pos.x;
    float dy   = goal.y - e->pos.y;
    float dist = sqrtf(dx * dx + dy * dy);
    float step = t->speed * dt;

    if (dist > 1.0f)
    {
        if (step > dist) step = dist;
        e->pos.x += dx / dist * step;
        e->pos.y += dy / dist * step;
    }
}

void enemy_damage(Game *g, Enemy *e, int dmg)
{
    if (!e->active) return;
    e->hp -= dmg;
    if (e->hp <= 0)
    {
        e->active = false;
        g->kills++;
        g->coins += g_enemyTypes[e->kind].reward;
    }
}

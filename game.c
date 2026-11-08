#include <string.h>
#include "game.h"
#include "maps.h"
#include "enemy.h"
#include "tower.h"
#include "projectile.h"
#include "path.h"
#include "ui.h"

// ---------------- setup ----------------
void game_init(Game *g)
{
    memset(g, 0, sizeof(*g));                    // clears every pool and flag at once
    g->state = STATE_MENU;
}

void game_start(Game *g, int mapIndex)
{
    memset(g, 0, sizeof(*g));
    g->state    = STATE_PLACE_CORE;
    g->mapIndex = mapIndex;
    g->map      = &g_maps[mapIndex];
    g->core.hp    = CORE_HP;
    g->core.maxHp = CORE_HP;
    g->coins      = COIN_START;
    g->selected   = TOWER_ARCHER;
}

// ---------------- helpers ----------------
bool game_mouse_tile(int *col, int *row)
{
    int mx = GetMouseX(), my = GetMouseY();
    if (mx < 0 || my < 0 || mx >= SCREEN_W || my >= SCREEN_H) return false;
    *col = mx / TILE;
    *row = my / TILE;
    return true;
}

bool game_can_place_core(const Game *g, int col, int row)
{
    if (col < CORE_MARGIN || col >= COLS - CORE_MARGIN) return false;
    if (row < CORE_MARGIN || row >= ROWS - CORE_MARGIN) return false;
    if (map_is_blocked(g->map, col, row)) return false;

    // the core must be reachable from the screen edges, or enemies could never arrive
    FlowField f;
    path_build(g->map, col, row, &f);
    return path_edge_reachable(g->map, &f);
}

void game_place_core(Game *g, int col, int row)
{
    g->core.col    = col;
    g->core.row    = row;
    g->core.pos    = (Vector2){ col * TILE + TILE / 2.0f, row * TILE + TILE / 2.0f };
    g->core.placed = true;
    g->occupied[row][col] = true;                // towers can't be built on it
    path_build(g->map, col, row, &g->flow);      // enemies follow this to the core
}

static void set_msg(Game *g, const char *text)
{
    g->msg      = text;
    g->msgTimer = 1.5f;
}

// keys 1/2/3 or a click on the build bar pick the tower type; a click on the map builds it
static void handle_build_input(Game *g)
{
    if (IsKeyPressed(KEY_ONE))   g->selected = TOWER_ARCHER;
    if (IsKeyPressed(KEY_TWO))   g->selected = TOWER_CANNON;
    if (IsKeyPressed(KEY_THREE)) g->selected = TOWER_DECOY;

    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return;

    int button = ui_build_hit(GetMousePosition());
    if (button >= 0)
    {
        g->selected = (TowerKind)button;
        return;
    }

    int c, r;
    if (!game_mouse_tile(&c, &r)) return;

    switch (tower_build_check(g, g->selected, c, r))
    {
        case BUILD_OK:       tower_place(g, g->selected, c, r);  break;
        case BUILD_NO_COINS: set_msg(g, "Not enough rupees");     break;
        case BUILD_CAP:      set_msg(g, "Tower limit reached");  break;
        default:             break;
    }
}

// ---------------- spawn director ----------------
// picks a random enemy kind among those unlocked at battle time t, weighted by type
static EnemyKind pick_kind(float t)
{
    int total = 0;
    for (int k = 0; k < ENEMY_KIND_COUNT; k++)
        if (t >= g_enemyTypes[k].unlockTime) total += g_enemyTypes[k].weight;

    int roll = GetRandomValue(0, total - 1);
    for (int k = 0; k < ENEMY_KIND_COUNT; k++)
    {
        if (t < g_enemyTypes[k].unlockTime) continue;
        if (roll < g_enemyTypes[k].weight) return (EnemyKind)k;
        roll -= g_enemyTypes[k].weight;
    }
    return ENEMY_GRUNT;
}

static void run_spawner(Game *g, float dt)
{
    float t        = g->battleTime;
    float progress = t / BATTLE_TIME;
    float interval = SPAWN_INTERVAL_START + (SPAWN_INTERVAL_END - SPAWN_INTERVAL_START) * progress;

    g->spawnTimer += dt;
    if (g->spawnTimer >= interval)
    {
        g->spawnTimer = 0.0f;
        enemy_spawn(g, pick_kind(t), -1);
    }

    if (!g->swarmDone && t >= SWARM_TIME)        // one big burst from a single side
    {
        g->swarmDone   = true;
        g->swarmBanner = 3.0f;
        int side = enemy_random_side(g);
        for (int i = 0; i < SWARM_SIZE; i++)
            enemy_spawn(g, pick_kind(t), side);
    }
}

// ---------------- battle ----------------
static void update_battle(Game *g, float dt)
{
    g->battleTime += dt;
    if (g->swarmBanner > 0.0f) g->swarmBanner -= dt;

    g->coinAcc += COIN_PER_SECOND * dt;          // passive income
    while (g->coinAcc >= 1.0f)
    {
        g->coins++;
        g->coinAcc -= 1.0f;
    }

    run_spawner(g, dt);

    for (int i = 0; i < MAX_ENEMIES; i++)
        if (g->enemies[i].active)
            enemy_update(g, &g->enemies[i], dt);

    for (int i = 0; i < MAX_TOWERS; i++)
        if (g->towers[i].active)
            tower_update(g, &g->towers[i], dt);

    for (int i = 0; i < MAX_PROJECTILES; i++)
        if (g->projectiles[i].active)
            projectile_update(g, &g->projectiles[i], dt);

    handle_build_input(g);

    if (g->core.hp <= 0)
    {
        g->core.hp = 0;
        g->state   = STATE_LOSE;
    }
    else if (g->battleTime >= BATTLE_TIME)       // survived: leftover enemies vanish
    {
        for (int i = 0; i < MAX_ENEMIES; i++)     g->enemies[i].active     = false;
        for (int i = 0; i < MAX_PROJECTILES; i++) g->projectiles[i].active = false;
        g->state = STATE_WIN;
    }
}

// ---------------- state machine ----------------
void game_update(Game *g, float dt)
{
    int c, r;

    if (g->msgTimer > 0.0f) g->msgTimer -= dt;

    if (g->state != STATE_MENU && IsKeyPressed(KEY_M))   // M = back to the menu any time
    {
        game_init(g);
        return;
    }

    switch (g->state)
    {
        case STATE_MENU:
        {
            int pick = -1;
            if (IsKeyPressed(KEY_ONE))   pick = 0;
            if (IsKeyPressed(KEY_TWO))   pick = 1;
            if (IsKeyPressed(KEY_THREE)) pick = 2;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                int hit = ui_menu_hit(GetMousePosition());
                if (hit >= 0) pick = hit;
            }
            if (pick >= 0 && pick < MAP_COUNT) game_start(g, pick);
            break;
        }

        case STATE_PLACE_CORE:
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) &&
                game_mouse_tile(&c, &r) && game_can_place_core(g, c, r))
            {
                game_place_core(g, c, r);
                g->state = STATE_PREP;
            }
            break;

        case STATE_PREP:                          // spend your starting coins, nothing moves yet
            handle_build_input(g);
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER))
            {
                g->battleTime = 0.0f;
                g->spawnTimer = 0.0f;
                g->state      = STATE_BATTLE;
            }
            break;

        case STATE_BATTLE:
            update_battle(g, dt);
            break;

        case STATE_WIN:
        case STATE_LOSE:
            if (IsKeyPressed(KEY_R))     game_start(g, g->mapIndex);   // replay the same map
            if (IsKeyPressed(KEY_ENTER)) game_init(g);                 // back to the menu
            break;
    }
}

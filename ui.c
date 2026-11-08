#include <stddef.h>
#include <math.h>
#include "ui.h"
#include "maps.h"
#include "enemy.h"
#include "tower.h"
#include "game.h"
#include "path.h"

static void text_center(const char *s, int cx, int y, int size, Color c)
{
    DrawText(s, cx - MeasureText(s, size) / 2, y, size, c);
}

// ---------------- menu ----------------
Rectangle ui_map_card(int i)
{
    return (Rectangle){ 84.0f, 200.0f + i * 150.0f, 600.0f, 125.0f };
}

int ui_menu_hit(Vector2 mouse)
{
    for (int i = 0; i < MAP_COUNT; i++)
        if (CheckCollisionPointRec(mouse, ui_map_card(i))) return i;
    return -1;
}

#define BTN_W   200
#define BTN_H   56
#define BTN_GAP 12

Rectangle ui_build_button(int i)
{
    float total = TOWER_KIND_COUNT * BTN_W + (TOWER_KIND_COUNT - 1) * BTN_GAP;
    float x0    = (SCREEN_W - total) / 2.0f;
    return (Rectangle){ x0 + i * (BTN_W + BTN_GAP), SCREEN_H - 90.0f, BTN_W, BTN_H };
}

int ui_build_hit(Vector2 mouse)
{
    for (int i = 0; i < TOWER_KIND_COUNT; i++)
        if (CheckCollisionPointRec(mouse, ui_build_button(i))) return i;
    return -1;
}

static void draw_menu(void)
{
    ClearBackground((Color){ 24, 32, 28, 255 });
    text_center("TOWER DEFENSE", SCREEN_W / 2, 60, 52, RAYWHITE);
    text_center(TextFormat("Protect your core for %d seconds", (int)BATTLE_TIME),
                SCREEN_W / 2, 125, 22, LIGHTGRAY);

    Vector2 mouse = GetMousePosition();
    for (int i = 0; i < MAP_COUNT; i++)
    {
        const MapDef *m = &g_maps[i];
        Rectangle r = ui_map_card(i);
        bool hot = CheckCollisionPointRec(mouse, r);

        DrawRectangleRec(r, hot ? (Color){ 50, 70, 60, 255 } : (Color){ 36, 50, 44, 255 });
        DrawRectangleLines((int)r.x, (int)r.y, (int)r.width, (int)r.height, hot ? GOLD : GRAY);

        // tiny preview of the map
        int ox = (int)r.x + 22, oy = (int)r.y + 22;
        for (int y = 0; y < ROWS; y++)
            for (int x = 0; x < COLS; x++)
                DrawRectangle(ox + x * 5, oy + y * 5, 5, 5,
                              m->tiles[y][x] == '#' ? m->blocked : m->grass);

        DrawText(TextFormat("%d. %s", i + 1, m->name), (int)r.x + 130, (int)r.y + 24, 30, RAYWHITE);
        DrawText(m->blurb, (int)r.x + 130, (int)r.y + 72, 18, LIGHTGRAY);
    }

    text_center("Press 1 / 2 / 3 or click a map", SCREEN_W / 2, 690, 20, GRAY);
}

// ---------------- world ----------------
static void draw_map(const Game *g)
{
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
        {
            Color c = (g->map->tiles[y][x] == '#') ? g->map->blocked : g->map->grass;
            DrawRectangle(x * TILE, y * TILE, TILE - 1, TILE - 1, c);
        }
}

// faint dots showing the routes enemies will take (drawn during prep so you can plan defenses)
static void draw_routes(const Game *g)
{
    for (int side = 0; side < 4; side++)
    {
        if (!(g->map->spawnSides & (1 << side))) continue;

        for (int k = 1; k < COLS; k += 3)          // sample every third tile along the edge
        {
            int c = (side == 3) ? 0 : (side == 1) ? COLS - 1 : k;
            int r = (side == 0) ? 0 : (side == 2) ? ROWS - 1 : k;
            if (g->flow.d[r][c] < 0) continue;

            for (int steps = 0; steps < ROWS * COLS; steps++)
            {
                DrawCircleV((Vector2){ c * TILE + TILE / 2.0f, r * TILE + TILE / 2.0f },
                            3.0f, Fade(WHITE, 0.2f));
                int nc, nr;
                if (!path_next(g->map, &g->flow, c, r, &nc, &nr)) break;
                c = nc;
                r = nr;
            }
        }
    }
}

static void draw_core(const Game *g)
{
    if (!g->core.placed) return;
    int x = g->core.col * TILE, y = g->core.row * TILE;

    DrawRectangle(x + 4, y + 4, TILE - 8, TILE - 8, GOLD);
    DrawRectangleLines(x + 4, y + 4, TILE - 8, TILE - 8, BROWN);
    DrawRectangle(x + 15, y + 15, TILE - 30, TILE - 30, MAROON);

    float frac = (float)g->core.hp / g->core.maxHp;
    DrawRectangle(x + 4, y - 2, TILE - 8, 5, DARKGRAY);
    DrawRectangle(x + 4, y - 2, (int)((TILE - 8) * frac), 5, LIME);
}

static void draw_towers(const Game *g)
{
    for (int i = 0; i < MAX_TOWERS; i++)
    {
        const Tower *t = &g->towers[i];
        if (!t->active) continue;
        const TowerType *tt = &g_towerTypes[t->kind];

        float lifeFrac = t->life / tt->life;
        float hpFrac   = (float)t->hp / tt->hp;
        int   bx  = t->col * TILE + TILE / 6;
        int   bw  = TILE * 2 / 3;
        int   top = t->row * TILE;

        Color body = (lifeFrac < 0.25f) ? Fade(tt->body, 0.5f) : tt->body;   // fades when about to vanish

        if (t->kind == TOWER_DECOY)               // a wide wall block
        {
            DrawRectangle(t->col * TILE + 4, top + 10, TILE - 8, TILE - 20, body);
            DrawRectangleLines(t->col * TILE + 4, top + 10, TILE - 8, TILE - 20, DARKGRAY);
        }
        else
        {
            DrawRectangle(bx, top + TILE / 6, bw, TILE * 2 / 3, body);
            if (t->kind == TOWER_CANNON) DrawCircleV(t->pos, TILE / 5.0f, ORANGE);
            else                         DrawCircleV(t->pos, TILE / 6.0f, SKYBLUE);
        }

        // health bar (top) and remaining-lifetime bar (bottom)
        DrawRectangle(bx, top + 2, bw, 4, DARKGRAY);
        DrawRectangle(bx, top + 2, (int)(bw * hpFrac), 4, LIME);
        DrawRectangle(bx, top + TILE - 7, bw, 3, DARKGRAY);
        DrawRectangle(bx, top + TILE - 7, (int)(bw * lifeFrac), 3, SKYBLUE);
    }
}

static void draw_enemies(const Game *g)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
    {
        const Enemy *e = &g->enemies[i];
        if (!e->active) continue;
        const EnemyType *t = &g_enemyTypes[e->kind];

        if (e->zap > 0.0f)                       // line showing the enemy's attack
            DrawLineEx(e->pos, e->zapTo, 3.0f, ORANGE);

        DrawCircleV(e->pos, t->radius, t->color);
        DrawCircleLines((int)e->pos.x, (int)e->pos.y, t->radius, BLACK);

        float frac = (float)e->hp / t->hp;
        int bw = (int)(t->radius * 2.0f);
        int bx = (int)(e->pos.x - t->radius);
        int by = (int)(e->pos.y - t->radius - 8);
        DrawRectangle(bx, by, bw, 4, DARKGRAY);
        DrawRectangle(bx, by, (int)(bw * frac), 4, LIME);
    }
}

static void draw_projectiles(const Game *g)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        const Projectile *p = &g->projectiles[i];
        if (!p->active) continue;
        if (p->splash > 0.0f) DrawCircleV(p->pos, 7.0f, ORANGE);   // cannonball
        else                  DrawCircleV(p->pos, 4.0f, YELLOW);
    }
}

static void draw_hover(const Game *g)
{
    int c, r;
    if (!game_mouse_tile(&c, &r)) return;

    if (g->state == STATE_PLACE_CORE)
    {
        bool ok = game_can_place_core(g, c, r);
        DrawRectangle(c * TILE + 4, r * TILE + 4, TILE - 8, TILE - 8, Fade(ok ? GOLD : RED, 0.5f));
        DrawRectangleLines(c * TILE, r * TILE, TILE, TILE, ok ? GREEN : RED);
    }
    else if (g->state == STATE_PREP || g->state == STATE_BATTLE)
    {
        if (ui_build_hit(GetMousePosition()) >= 0) return;   // pointing at the build bar

        const TowerType *tt = &g_towerTypes[g->selected];
        bool ok = tower_can_build(g, g->selected, c, r);

        if (ok) DrawRectangle(c * TILE + 6, r * TILE + 6, TILE - 12, TILE - 12, Fade(tt->body, 0.5f));
        DrawRectangleLines(c * TILE, r * TILE, TILE, TILE, ok ? GREEN : RED);
        if (ok && tt->range > 0.0f)   // preview the range of the tower you are about to place
            DrawCircleLines(c * TILE + TILE / 2, r * TILE + TILE / 2, tt->range, Fade(WHITE, 0.5f));
    }
}

// ---------------- HUD ----------------
static void draw_build_bar(const Game *g)
{
    for (int i = 0; i < TOWER_KIND_COUNT; i++)
    {
        const TowerType *tt = &g_towerTypes[i];
        Rectangle r = ui_build_button(i);
        int x = (int)r.x, y = (int)r.y;
        bool selected = (g->selected == (TowerKind)i);
        bool afford   = (g->coins >= tt->cost);

        DrawRectangleRec(r, Fade(BLACK, 0.7f));
        DrawRectangleLines(x, y, (int)r.width, (int)r.height, selected ? GOLD : GRAY);
        DrawRectangle(x + 8, y + 8, 14, 14, tt->body);

        DrawText(TextFormat("[%d] %s", i + 1, tt->name), x + 28, y + 6, 18, afford ? RAYWHITE : GRAY);
        const char *cost = TextFormat("Rs %d", tt->cost);
        DrawText(cost, x + (int)r.width - MeasureText(cost, 18) - 8, y + 6, 18, afford ? GOLD : RED);
        DrawText(TextFormat("%s  HP %d", tt->desc, tt->hp), x + 8, y + 33, 14, LIGHTGRAY);
    }
}

static void draw_hud(const Game *g)
{
    DrawRectangle(0, 0, SCREEN_W, 40, Fade(BLACK, 0.55f));
    DrawText(TextFormat("%s   Rs %d", g->map->name, g->coins), 10, 10, 20, GOLD);

    // timer, centered (shows the full time during prep)
    float remaining = BATTLE_TIME - g->battleTime;
    if (remaining < 0.0f) remaining = 0.0f;
    int s = (int)ceilf(remaining);
    Color timeColor = (g->state == STATE_BATTLE && remaining <= 10.0f) ? RED : WHITE;
    text_center(TextFormat("%d:%02d", s / 60, s % 60), SCREEN_W / 2, 5, 32, timeColor);

    const char *tw = TextFormat("Towers %d/%d", tower_count(g), MAX_ACTIVE_TOWERS);
    DrawText(tw, SCREEN_W - MeasureText(tw, 20) - 10, 10, 20, SKYBLUE);

    // hint line
    const char *hint = NULL;
    if (g->state == STATE_PLACE_CORE) hint = "Click a tile to place your CORE (not too close to the edge)";
    if (g->state == STATE_PREP)       hint = "Dots show enemy routes. Pick tower 1/2/3, click to build, SPACE = start";
    if (hint)
    {
        DrawRectangle(0, 44, SCREEN_W, 28, Fade(BLACK, 0.55f));
        text_center(hint, SCREEN_W / 2, 49, 20, RAYWHITE);
    }

    if (g->swarmBanner > 0.0f)
        text_center("!! SWARM INCOMING !!", SCREEN_W / 2, 90, 36, RED);

    if (g->state == STATE_PREP || g->state == STATE_BATTLE)
    {
        draw_build_bar(g);
        if (g->msgTimer > 0.0f && g->msg)
            text_center(g->msg, SCREEN_W / 2, SCREEN_H - 118, 22, ORANGE);
    }

    // core health bar along the bottom
    if (g->core.placed)
    {
        int bw = 300, bx = (SCREEN_W - bw) / 2, by = SCREEN_H - 24;
        float frac = (float)g->core.hp / g->core.maxHp;
        DrawText("CORE", bx - 56, by - 2, 18, GOLD);
        DrawRectangle(bx, by, bw, 14, DARKGRAY);
        DrawRectangle(bx, by, (int)(bw * frac), 14, frac > 0.3f ? LIME : RED);
    }
}

static void draw_end_overlay(const Game *g)
{
    bool win = (g->state == STATE_WIN);
    DrawRectangle(0, 0, SCREEN_W, SCREEN_H, Fade(BLACK, 0.6f));
    text_center(win ? "YOU SURVIVED!" : "CORE DESTROYED", SCREEN_W / 2, 270, 54, win ? GOLD : RED);
    text_center(TextFormat("Enemies defeated: %d", g->kills), SCREEN_W / 2, 345, 26, RAYWHITE);
    text_center("ENTER - map menu        R - replay this map", SCREEN_W / 2, 400, 22, LIGHTGRAY);
}

// ---------------- entry point ----------------
void ui_draw(const Game *g)
{
    if (g->state == STATE_MENU)
    {
        draw_menu();
        return;
    }

    draw_map(g);
    if (g->state == STATE_PREP) draw_routes(g);
    draw_core(g);
    draw_towers(g);
    draw_enemies(g);
    draw_projectiles(g);
    draw_hover(g);
    draw_hud(g);

    if (g->state == STATE_WIN || g->state == STATE_LOSE)
        draw_end_overlay(g);
}

#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include "raylib.h"
#include "config.h"

typedef enum {
    STATE_MENU,
    STATE_PLACE_CORE,
    STATE_PREP,
    STATE_BATTLE,
    STATE_WIN,
    STATE_LOSE
} GameState;

// what an enemy is attacking: a tower index (>= 0), or one of these
#define TARGET_NONE (-1)
#define TARGET_CORE (-2)

// screen edges enemies may spawn from (bit flags for MapDef.spawnSides)
#define SIDE_TOP    (1 << 0)
#define SIDE_RIGHT  (1 << 1)
#define SIDE_BOTTOM (1 << 2)
#define SIDE_LEFT   (1 << 3)
#define SIDE_ALL    (SIDE_TOP | SIDE_RIGHT | SIDE_BOTTOM | SIDE_LEFT)

typedef enum { ENEMY_GRUNT, ENEMY_RUNNER, ENEMY_TANK, ENEMY_KIND_COUNT } EnemyKind;

typedef enum {
    BEHAVIOR_NEAREST,       // attacks the closest building (tower or core)
    BEHAVIOR_CORE_ONLY,     // ignores towers, rushes the core
    BEHAVIOR_TOWERS_FIRST   // smashes towers in range, core only if no tower is near
} EnemyBehavior;

typedef struct {
    const char   *name;
    int           hp;
    float         speed;        // pixels per second
    int           damage;       // per attack
    float         range;        // attack range in pixels
    float         cooldown;     // seconds between attacks
    float         radius;       // drawn size
    Color         color;
    float         unlockTime;   // battle second at which this type starts spawning
    int           weight;       // spawn chance relative to other unlocked types
    EnemyBehavior behavior;
    int           reward;       // coins earned for killing it
} EnemyType;

typedef struct {
    Vector2   pos;        // current position in pixels
    EnemyKind kind;
    int       hp;
    float     atkTimer;   // counts up toward its next attack
    float     zap;        // seconds left to show its attack line
    Vector2   zapTo;      // where that attack line points
    unsigned  id;         // unique id, so projectiles can tell slot reuse apart
    bool      active;
} Enemy;

typedef enum { TOWER_ARCHER, TOWER_CANNON, TOWER_DECOY, TOWER_KIND_COUNT } TowerKind;

typedef struct {
    const char *name;
    const char *desc;       // short role text for the build bar
    int         cost;       // coins
    int         hp;
    float       life;       // seconds before it vanishes on its own
    float       range;      // pixels
    float       cooldown;   // seconds between shots
    int         damage;     // 0 = never attacks (decoy)
    float       splash;     // explosion radius in pixels, 0 = single target
    float       projSpeed;  // pixels per second
    Color       body;
} TowerType;

typedef struct {
    int       col, row;
    Vector2   pos;          // center of the tile in pixels
    TowerKind kind;
    float     timer;        // counts up toward the next shot
    int       hp;
    float     life;         // seconds left before it vanishes
    bool      active;
} Tower;

typedef struct {
    Vector2  pos;
    int      target;      // index into enemies[]
    unsigned targetId;    // the id that enemy had when we aimed (must still match)
    float    speed;
    int      damage;
    float    splash;      // > 0: explodes and hurts everything within this radius
    bool     active;
} Projectile;

typedef struct {
    int     col, row;
    Vector2 pos;
    int     hp, maxHp;
    bool    placed;
} Core;

typedef struct {
    const char *name;
    const char *blurb;
    char        tiles[ROWS][COLS + 1];   // '.' = buildable, '#' = blocked (rock/water)
    Color       grass, blocked;
    int         spawnSides;              // SIDE_* flags
} MapDef;

// walking cost from every tile to the core (-1 = unreachable); filled by path_build()
typedef struct { int d[ROWS][COLS]; } FlowField;

// everything that changes during play lives here, so no global variables
typedef struct {
    GameState     state;
    int           mapIndex;
    const MapDef *map;
    Core          core;
    FlowField     flow;          // enemy routes, built when the core is placed
    Enemy         enemies[MAX_ENEMIES];
    Tower         towers[MAX_TOWERS];
    Projectile    projectiles[MAX_PROJECTILES];
    bool          occupied[ROWS][COLS];
    float         battleTime;
    float         spawnTimer;
    bool          swarmDone;
    float         swarmBanner;   // seconds left to show the swarm warning
    unsigned      nextEnemyId;
    int           kills;
    int           coins;
    float         coinAcc;       // fractional coins from passive income
    TowerKind     selected;      // tower type the next click builds
    const char   *msg;           // short warning shown above the build bar
    float         msgTimer;
} Game;

#endif

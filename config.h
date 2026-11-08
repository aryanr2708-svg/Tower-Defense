#ifndef CONFIG_H
#define CONFIG_H

// ---------------- tuning: change these to rebalance the game ----------------
#define TILE 48
#define COLS 16
#define ROWS 16
#define SCREEN_W (COLS * TILE)
#define SCREEN_H (ROWS * TILE)

// match
#define BATTLE_TIME          60.0f   // seconds you must survive
#define CORE_HP              100
#define CORE_MARGIN          2       // core can't be placed this close to the screen edge

// spawn director (spawn interval shrinks from START to END over the battle)
#define SPAWN_INTERVAL_START 2.0f
#define SPAWN_INTERVAL_END   0.6f
#define SWARM_TIME           45.0f   // one big burst of enemies at this time
#define SWARM_SIZE           8

// economy
#define COIN_START           200
#define COIN_PER_SECOND      6.0f    // passive income, only while the battle runs

// towers (cost, hp, damage... per type live in the table at the top of tower.c)
#define MAX_ACTIVE_TOWERS    8

// pool sizes
#define MAX_ENEMIES          100
#define MAX_TOWERS           16
#define MAX_PROJECTILES      200
#define MAP_COUNT            3

#endif

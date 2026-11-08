#ifndef ENEMY_H
#define ENEMY_H

#include "types.h"

extern const EnemyType g_enemyTypes[ENEMY_KIND_COUNT];

// picks a random screen edge allowed by the map that can reach the core (0..3), or -1 if none
int  enemy_random_side(const Game *g);

// side: 0 top, 1 right, 2 bottom, 3 left, or -1 for a random allowed side
void enemy_spawn(Game *g, EnemyKind kind, int side);

void enemy_update(Game *g, Enemy *e, float dt);

// the single place enemies take damage: handles death, kill count and coin reward
void enemy_damage(Game *g, Enemy *e, int dmg);

#endif

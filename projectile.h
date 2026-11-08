#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "types.h"

void projectile_shoot(Game *g, Vector2 from, int enemyIndex, const TowerType *tt);
void projectile_update(Game *g, Projectile *p, float dt);

#endif

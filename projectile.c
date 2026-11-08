#include <math.h>
#include "projectile.h"
#include "enemy.h"

void projectile_shoot(Game *g, Vector2 from, int enemyIndex, const TowerType *tt)
{
    for (int i = 0; i < MAX_PROJECTILES; i++)
    {
        Projectile *p = &g->projectiles[i];
        if (p->active) continue;

        p->pos      = from;
        p->target   = enemyIndex;
        p->targetId = g->enemies[enemyIndex].id;
        p->speed    = tt->projSpeed;
        p->damage   = tt->damage;
        p->splash   = tt->splash;
        p->active   = true;
        return;
    }
}

void projectile_update(Game *g, Projectile *p, float dt)
{
    Enemy *e = &g->enemies[p->target];

    // target died, left, or its slot was reused by a different enemy
    if (!e->active || e->id != p->targetId)
    {
        p->active = false;
        return;
    }

    float dx   = e->pos.x - p->pos.x;
    float dy   = e->pos.y - p->pos.y;
    float dist = sqrtf(dx * dx + dy * dy);
    float step = p->speed * dt;

    if (dist <= step)                      // reached the enemy: hit
    {
        if (p->splash > 0.0f)              // explosion: hurt every enemy near the impact
        {
            Vector2 impact = e->pos;
            float   r2     = p->splash * p->splash;
            for (int i = 0; i < MAX_ENEMIES; i++)
            {
                Enemy *o = &g->enemies[i];
                if (!o->active) continue;
                float ox = o->pos.x - impact.x, oy = o->pos.y - impact.y;
                if (ox * ox + oy * oy <= r2)
                    enemy_damage(g, o, p->damage);
            }
        }
        else
        {
            enemy_damage(g, e, p->damage);
        }
        p->active = false;
    }
    else
    {
        p->pos.x += dx / dist * step;
        p->pos.y += dy / dist * step;
    }
}

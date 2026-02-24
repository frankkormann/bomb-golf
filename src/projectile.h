/*
 * Manage a single projectile. The projectile's behaviors are defined by
 * which Projectile is used.
 */

#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <stdbool.h>
#include <3ds.h>
#include "rendering/spritesheet.h"

typedef struct projectile *Projectile;

#include "projectiles/ball.h"

/*
 * Sets the projectile to act like proj. Implicitly calls Projectile_Reset().
 */
void Projectile_SetType(Projectile proj);

void Projectile_SetPos(float x, float y);
void Projectile_GetPos(float *x, float *y);
bool Projectile_IsMoving();

/*
 * Resets the projectile based on its current type.
 */
void Projectile_Reset();

void Projectile_Launch(float velX, float velY);

/*
 * Call this once per physics frame.
 */
void Projectile_Update();

/*
 * Call this once per graphics frame.
 */
void Projectile_Draw(float depth);

/*
 * Uses C2D_ViewTranslate to center the view on the projectile.
 */
void Projectile_CenterViewC2D(gfxScreen_t screen);

#endif

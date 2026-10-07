/*
 * Projectile which travels in a straightish line, ignoring gravity but
 * guided by the player, until it hits terrain.
 *
 * Starts in the missile state where it moves in a straight line. Transitions
 * to the ball state after hitting something or when exploded by the player.
 * Clears a circle of terrain after exploding.
 */

#ifndef MISSILE_H
#define MISSILE_H

#include "../projectile.h"

extern Projectile projectileMissile;

#endif

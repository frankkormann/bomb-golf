/*
 * Projectile which burrows through the ground and explodes on the way out.
 *
 * Starts in the drill state. Once it hits terrain, starts burrowing through
 * it in a straight line, bouncing off the edges of the screen. While
 * underground, the player can tap to explode it and transition it to a normal
 * ball; also transitions to a ball once it exits the ground.
 */

#ifndef DRILL_H
#define DRILL_H

#include "../projectile.h"

extern Projectile projectileDrill;

#endif

/*
 * Projectile which transitions between a ball and bomb state.
 *
 * Starts in the bomb state. Transitions to the ball state after hitting
 * ground or when exploded by the player. Clears a circle of terrain durng
 * each transition.
 *
 * In the ball state, bounces off of ground like a bouncy ball.
 */

#ifndef BALL_H
#define BALL_H

#include "../projectile.h"

extern Projectile projectileBall;

#endif

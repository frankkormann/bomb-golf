/*
 * Animation of a firework rocket flying up until it hits something, then
 * exploding.
 */

#ifndef FIREWORKS_H
#define FIREWORKS_H

typedef struct {
	float startX;
	float startY;
} Firework_Params;

#include "../animation.h"

extern Animation animationFirework;

Animation_Params Firework_MakeParams (float x, float y);

#endif

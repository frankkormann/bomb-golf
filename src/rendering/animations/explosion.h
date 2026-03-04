#ifndef EXPLOSION_H
#define EXPLOSION_H

typedef struct {
	float x;
	float y;
	float radius;
} Explosion_Params;

#include "../animation.h"

extern Animation animationExplosion;

Animation_Params Explosion_MakeParams(float x, float y, float radius);

#endif

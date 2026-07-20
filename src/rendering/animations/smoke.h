/*
 * Animation of a puff of smoke that quickly disappears.
 */

#ifndef SMOKE_H
#define SMOKE_H

typedef struct {
	float x;
	float y;
} Smoke_Params;

#include "../animation.h"

extern Animation animationSmoke;

Animation_Params Smoke_MakeParams(float x, float y);

#endif

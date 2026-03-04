/*
 * Manage playing back Animations.
 */

#ifndef ANIMATION_H
#define ANIMATION_H

#include <stdbool.h>

typedef struct animation *Animation;
typedef union animation_params Animation_Params;

#include "animations/explosion.h"

union animation_params {
	Explosion_Params explosion;
};

/*
 * Starts playing anim using params. When it ends, calls onFinish if onFinish
 * isn't NULL.
 * Each implementor of Animation should provide a function to make their params.
 *
 * Note: onFinish will be stored for later use, so it must not become dangling.
 *
 * Returns false if an error occured in starting the animation.
 */
bool Animation_Start(Animation anim, Animation_Params params, void (*onFinish)(void));

/*
 * Call this once per frame.
 */
void Animation_Update();

/*
 * Removes playing all animations. Calls each one's onFinish callback if
 * doCallbacks is set.
 */
void Animation_Clear(bool doCallbacks);

void Animation_Draw();

#endif

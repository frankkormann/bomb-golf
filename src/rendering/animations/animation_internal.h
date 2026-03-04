/*
 * Definitions for implementors of an Animation.
 */

#ifndef ANIMATION_INTERNAL_H
#define ANIMATION_INTERNAL_H

#include <stdbool.h>
#include "../animation.h"

typedef struct {
	Animation anim;
	void (*onFinish)(void);
	void *data;
} AnimationI_AnimObj;

typedef struct {
	bool success;
	AnimationI_AnimObj obj;
} AnimationI_CreateAnimReturnValue;

struct animation {
	/*
	 * Creates an AnimationI_AnimObj for params and allocates animation-
	 * specific data. Does not need to fill in anim or onFinish.
	 */
	AnimationI_CreateAnimReturnValue (*const create)(Animation_Params params);
	/*
	 * Updates obj for one frame.
	 */
	void (*const update)(AnimationI_AnimObj obj);
	/*
	 * Draws obj.
	 */
	void (*const draw)(AnimationI_AnimObj obj);
	/*
	 * Returns true if obj has completed its animation.
	 */
	bool (*const isFinished)(AnimationI_AnimObj obj);
	/*
	 * Frees any resources used by obj.
	 */
	void (*const free)(AnimationI_AnimObj obj);
};

#endif

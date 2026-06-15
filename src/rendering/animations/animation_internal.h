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

struct animation {
	/*
	 * Creates Animation-specific data and fills it into obj. Does not need
	 * to fill other fields in obj.
	 */
	bool (*const create)(Animation_Params params, AnimationI_AnimObj *obj);
	/*
	 * Updates obj for one frame.
	 */
	void (*const update)(AnimationI_AnimObj *obj);
	/*
	 * Draws obj.
	 */
	void (*const draw)(AnimationI_AnimObj *obj);
	/*
	 * Returns true if obj has completed its animation.
	 */
	bool (*const isFinished)(AnimationI_AnimObj *obj);
	/*
	 * Frees any resources used by obj.
	 */
	void (*const free)(AnimationI_AnimObj *obj);
};

#endif

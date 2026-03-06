#include <stddef.h>
#include <stdbool.h>
#include "animation.h"
#include "animations/animation_internal.h"

#define CONCURRENT_ANIMATIONS 4

// Empty slots are denoted by struct member anim = NULL
static AnimationI_AnimObj animationObjs[CONCURRENT_ANIMATIONS];

bool Animation_Start(Animation anim, Animation_Params params, void (*onFinish)(void)) {
	size_t i = 0;
	while (i < CONCURRENT_ANIMATIONS) {
		if (!animationObjs[i].anim) break;
		i++;
	}
	if (i >= CONCURRENT_ANIMATIONS) return false;

	AnimationI_CreateAnimReturnValue rv = anim->create(params);
	if (!rv.success) return false;

	animationObjs[i] = rv.obj;
	animationObjs[i].anim = anim;
	animationObjs[i].onFinish = onFinish;
	return true;
}

void Animation_Update() {
	for (size_t i = 0; i < CONCURRENT_ANIMATIONS; i++) {
		Animation anim = animationObjs[i].anim;
		if (!anim) continue;

		anim->update(&animationObjs[i]);

		if (anim->isFinished(&animationObjs[i])) {
			if (animationObjs[i].onFinish) {
				animationObjs[i].onFinish();
			}
			anim->free(&animationObjs[i]);
			animationObjs[i].anim = NULL;
		}
	}
}

void Animation_Clear(bool doCallbacks) {
	for (size_t i = 0; i < CONCURRENT_ANIMATIONS; i++) {
		Animation anim = animationObjs[i].anim;
		if (!anim) continue;

		if (doCallbacks && animationObjs[i].onFinish) {
			animationObjs[i].onFinish();
		}
		anim->free(&animationObjs[i]);
		animationObjs[i].anim = NULL;
	}
}

void Animation_Draw() {
	for (size_t i = 0; i < CONCURRENT_ANIMATIONS; i++) {
		if (!animationObjs[i].anim) continue;
		animationObjs[i].anim->draw(&animationObjs[i]);
	}
}

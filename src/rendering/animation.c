#include <stddef.h>
#include <malloc.h>
#include <stdbool.h>
#include "animation.h"
#include "animations/animation_internal.h"
#include "../util/list.h"

static List activeAnimations;

bool Animation_Init() {
	activeAnimations = List_Create();
	if (!activeAnimations) return false;

	return true;
}

void Animation_Exit() {
	Animation_Clear(false);
	List_Free(activeAnimations);
}

bool Animation_Start(Animation anim, Animation_Params params,
		void (*onFinish)(void)) {
	AnimationI_AnimObj *obj = malloc(sizeof(*obj));
	if (!obj) return false;

	if (!anim->create(params, obj)) {
		free(obj);
		return false;
	}

	obj->anim = anim;
	obj->onFinish = onFinish;

	if (List_Push(activeAnimations, obj)) {
		return true;
	} else {
		anim->free(obj);
		free(obj);
		return false;
	}
}

void Animation_Update() {
	void update(void *animationObj) {
		AnimationI_AnimObj *obj = animationObj;
		Animation anim = obj->anim;
		anim->update(obj);
		if (anim->isFinished(obj) && obj->onFinish) {
			obj->onFinish();
		}
	}
	bool isFinished(void *animationObj) {
		AnimationI_AnimObj *obj = animationObj;
		return obj->anim->isFinished(obj);
	}
	void freeAnim(void *animationObj) {
		AnimationI_AnimObj *obj = animationObj;
		obj->anim->free(obj);
		free(obj);
	}

	List_ForEach(activeAnimations, update);
	List_Filter(activeAnimations, isFinished, freeAnim);
}

void Animation_Clear(bool doCallbacks) {
	bool returnTrue() { return true; }
	void doOnFinish(void *animationObj) {
		AnimationI_AnimObj *obj = animationObj;
		if (obj->onFinish) obj->onFinish();
	}
	void freeAnim(void *animationObj) {
		AnimationI_AnimObj *obj = animationObj;
		obj->anim->free(obj);
		free(obj);
	}

	if (doCallbacks) List_ForEach(activeAnimations, doOnFinish);
	List_Filter(activeAnimations, returnTrue, freeAnim);
}

void Animation_Draw() {
	void draw(void *animationObj) {
		AnimationI_AnimObj *obj = animationObj;
		obj->anim->draw(obj);
	}
	List_ForEach(activeAnimations, draw);
}

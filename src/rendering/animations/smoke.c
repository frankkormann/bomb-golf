#include <stdlib.h>
#include "../animation.h"
#include "animation_internal.h"
#include "smoke.h"
#include "../spritesheet.h"
#include "../rendertarget.h"
#include "../../scenes/course.h"

#define ANIMATION_FRAME_LENGTH 15
#define ANIMATION_LENGTH \
	((SPRITE_SMOKE3 - SPRITE_SMOKE1 + 1) * ANIMATION_FRAME_LENGTH)

typedef struct {
	float x;
	float y;
	float frame;
	bool flipHoriz;
	bool flipVert;
} SmokeData;

Animation_Params Smoke_MakeParams(float x, float y) {
	return (Animation_Params) { .smoke = { x, y } };
}

static bool create(Animation_Params params, AnimationI_AnimObj *obj) {
	SmokeData *data = malloc(sizeof(*data));
	if (!data) return false;

	data->x = params.smoke.x;
	data->y = params.smoke.y;	
	data->frame = 0;
	data->flipHoriz = rand() % 2 == 0;
	data->flipVert = rand() % 2 == 0;

	obj->data = data;

	return true;
}

static void update(AnimationI_AnimObj *obj, float timestep) {
	((SmokeData*)obj->data)->frame += timestep;
}

static void draw(AnimationI_AnimObj *obj, float depth) {
	SmokeData *data = (SmokeData*)obj->data;
	SpriteSheet_Sprite sprite = SPRITE_SMOKE1 
			+ (data->frame / ANIMATION_FRAME_LENGTH);
	SpriteSheet_DrawCentered(sprite, (int)data->x, (int)data->y, depth, 0,
			data->flipHoriz, data->flipVert);
}

static bool isFinished(AnimationI_AnimObj *obj) {
	return ((SmokeData*)obj->data)->frame >= ANIMATION_LENGTH;
}

static void _free(AnimationI_AnimObj *obj) {
	free(obj->data);
}

Animation animationSmoke = &(struct animation) {
	.create = create,
	.update = update,
	.draw = draw,
	.isFinished = isFinished,
	.free = _free
};

#include <malloc.h>
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
	int frame;
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

	obj->data = data;

	return true;
}

static void update(AnimationI_AnimObj *obj) {
	((SmokeData*)obj->data)->frame++;
}

static void draw(AnimationI_AnimObj *obj) {
	C2D_SceneBegin(RenderTarget_GetBottom());
	C2D_ViewTranslate(-Course_GetScreenOffset(), 0);

	SmokeData *data = (SmokeData*)obj->data;
	SpriteSheet_Sprite sprite = SPRITE_SMOKE1 
			+ (data->frame / ANIMATION_FRAME_LENGTH);
	SpriteSheet_DrawCentered(sprite, (int)data->x, (int)data->y, 0.5, 0, false,
			false);

	C2D_ViewReset();
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

#include <citro2d.h>
#include <malloc.h>
#include "../animation.h"
#include "animation_internal.h"
#include "explosion.h"
#include "../colors.h"
#include "../rendertarget.h"
#include "../../scenes/course.h"

#define EXPLOSION_STAGE_1 0
#define EXPLOSION_STAGE_2 20
#define EXPLOSION_FINISHED 40

typedef struct {
	float x;
	float y;
	float radius;
	int counter;
} ExplosionData;

Animation_Params Explosion_MakeParams(float x, float y, float radius) {
	return (Animation_Params) { .explosion = { x, y, radius } };
}

static AnimationI_CreateAnimReturnValue create(Animation_Params params) {
	ExplosionData *data = malloc(sizeof(*data));
	if (!data) {
		return (AnimationI_CreateAnimReturnValue) { .success = false };
	}

	data->x = params.explosion.x;
	data->y = params.explosion.y;
	data->radius = params.explosion.radius;
	data->counter = 0;

	return (AnimationI_CreateAnimReturnValue) {
		.success = true,
		.obj = { .data = data }
	};
}

static void update(AnimationI_AnimObj *obj) {
	((ExplosionData*)obj->data)->counter++;
}

static void draw(AnimationI_AnimObj *obj) {
	C2D_SceneBegin(RenderTarget_GetBottom());
	C2D_ViewTranslate(-Course_GetScreenOffset(), 0);

	ExplosionData *data = (ExplosionData*)obj->data;
	u32 color = data->counter > EXPLOSION_STAGE_2 ? COLOR_ORANGE : COLOR_RED;
	C2D_DrawCircleSolid(data->x, data->y, 0.5, data->radius, color);

	C2D_ViewReset();
}

static bool isFinished(AnimationI_AnimObj *obj) {
	return ((ExplosionData*)obj->data)->counter >= EXPLOSION_FINISHED;
}

void _free(AnimationI_AnimObj *obj) {
	free(obj->data);
}

Animation animationExplosion = &(struct animation) {
	.create = create,
	.update = update,
	.draw = draw,
	.isFinished = isFinished,
	.free = _free
};

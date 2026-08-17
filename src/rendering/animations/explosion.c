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
	float counter;
} ExplosionData;

Animation_Params Explosion_MakeParams(float x, float y, float radius) {
	return (Animation_Params) { .explosion = { x, y, radius } };
}

static bool create(Animation_Params params, AnimationI_AnimObj *obj) {
	ExplosionData *data = malloc(sizeof(*data));
	if (!data) return false;

	data->x = params.explosion.x;
	data->y = params.explosion.y;
	data->radius = params.explosion.radius;
	data->counter = 0;

	obj->data = data;
	return true;
}

static void update(AnimationI_AnimObj *obj, float timestep) {
	((ExplosionData*)obj->data)->counter += timestep;
}

static void draw(AnimationI_AnimObj *obj, float depth) {
	ExplosionData *data = (ExplosionData*)obj->data;
	u32 color = data->counter > EXPLOSION_STAGE_2 ? COLOR_ORANGE : COLOR_RED;
	C2D_DrawCircleSolid(data->x, data->y, depth, data->radius, color);
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

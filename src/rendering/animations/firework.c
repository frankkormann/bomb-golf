#include <citro2d.h>
#include <malloc.h>
#include <stdbool.h>
#include "../animation.h"
#include "animation_internal.h"
#include "firework.h"
#include "../colors.h"
#include "../rendertarget.h"
#include "../spritesheet.h"
#include "../../scenes/course.h"

#define ANIMATION_FRAME_LENGTH 5
#define ANIMATION_LENGTH \
	(((SPRITE_FIREWORK_12 - SPRITE_FIREWORK_1) * ANIMATION_FRAME_LENGTH) + 20)

#define TRAIL_LENGTH 8
// Width and height
#define TRAIL_PARTICLE_SIZE 2

// Explode once the rocket reaches this height
#define EXPLOSION_Y 50
#define EXPLOSION_RADIUS 25

typedef struct {
	float x;
	float y;
} Point;

typedef struct {
	Point loc;
	bool exploding;
	int explosionFrame;
	Point trail[TRAIL_LENGTH];
	int oldestTrailParticle;
} FireworkData;

Animation_Params Firework_MakeParams(float x, float y) {
	return (Animation_Params) { .firework = { x, y } };
}

static AnimationI_CreateAnimReturnValue create(Animation_Params params) {
	FireworkData *data = malloc(sizeof(*data));
	if (!data) {
		return (AnimationI_CreateAnimReturnValue) { .success = false };
	}

	data->loc.x = params.firework.startX;
	data->loc.y = params.firework.startY;
	data->exploding = false;
	data->explosionFrame = 0;
	for (size_t i = 0; i < TRAIL_LENGTH; i++) {
		data->trail[i] = (Point) { -1, -1 };
	}
	data->oldestTrailParticle = 0;

	return (AnimationI_CreateAnimReturnValue) {
		.success = true,
		.obj = { .data = data }
	};
}

static SpriteSheet_Sprite getSpriteForFrame(int frame) {
	return (frame / ANIMATION_FRAME_LENGTH) + SPRITE_FIREWORK_1;
}

static void update(AnimationI_AnimObj obj) {
	FireworkData *data = (FireworkData*)obj.data;

	data->trail[data->oldestTrailParticle] = data->exploding ?
			(Point) { -1, -1 } : data->loc;
	data->oldestTrailParticle = (data->oldestTrailParticle + 1) % TRAIL_LENGTH;

	if (data->exploding) {
		data->explosionFrame++;
	} else {
		data->loc.y--;
	}
	
	if (data->loc.y < EXPLOSION_Y
			|| Course_CheckTerrain(data->loc.x, data->loc.y)) {
		data->exploding = true;
		Course_ClearCircle(data->loc.x, data->loc.y, EXPLOSION_RADIUS);
	}
}

static void draw(AnimationI_AnimObj obj) {
	C2D_SceneBegin(RenderTarget_GetBottom());
	C2D_ViewTranslate(-Course_GetScreenOffset(), 0);

	FireworkData *data = (FireworkData*)obj.data;
	if (data->exploding) {
		SpriteSheet_Sprite spr = getSpriteForFrame(data->explosionFrame);
		if (spr <= SPRITE_FIREWORK_12) {
			SpriteSheet_Draw(spr, data->loc.x, data->loc.y, 0.5, 0,
					false, false, NULL);
		}
	} else {
		C2D_DrawRectSolid(data->loc.x, data->loc.y, 0.5, TRAIL_PARTICLE_SIZE,
				TRAIL_PARTICLE_SIZE, COLOR_WHITE);
	}

	for (size_t i = 0; i < TRAIL_LENGTH; i++) {
		C2D_DrawRectSolid(data->trail[i].x, data->trail[i].y, 0.5,
				TRAIL_PARTICLE_SIZE, TRAIL_PARTICLE_SIZE,
				COLOR_ORANGE);
	}

	C2D_ViewReset();
}

static bool isFinished(AnimationI_AnimObj obj) {
	return ((FireworkData*)obj.data)->explosionFrame > ANIMATION_LENGTH;
}

static void _free(AnimationI_AnimObj obj) {
	free(obj.data);
}

Animation animationFirework = &(struct animation) {
	.create = create,
	.update = update,
	.draw = draw,
	.isFinished = isFinished,
	.free = _free
};

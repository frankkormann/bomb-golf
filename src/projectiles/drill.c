#include <stdbool.h>
#include <3ds.h>
#include <citro2d.h>
#include "../projectile.h"
#include "projectile_internal.h"
#include "drill.h"
#include "missile.h"
#include "../environment/terrain.h"
#include "../rendering/color.h"

#define BALL_RADIUS 4

static void reset() {
	ProjDefault_Reset();
}

static void launch(float velX, float velY) {
	ProjDefault_Launch(velX, velY);
}

static bool move(float timestep, float *hitX, float *hitY,
			Terrain_Type *hitType) {
	return ProjDefault_Move(timestep, hitX, hitY, hitType);
}

static void onHitGround(float hitX, float hitY, Terrain_Type hitType) {
	ProjDefault_OnHitGround(hitX, hitY, hitType);
}

static void draw(float depth) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	C2D_DrawRectSolid(data->x - 4, data->y - 4, depth, 8, 8, COLOR_DRED);
}

Projectile projectileDrill = &(struct projectile) {
	.radius =	BALL_RADIUS,
	.reset =	reset,
	.launch =	launch,
	.move =		move,
	.isMoving =	ProjDefault_IsMoving,
	.onHitGround =	onHitGround,
	.draw =		draw
};

#include <math.h>
#include <citro2d.h>
#include "../projectile.h"
#include "projectile_internal.h"
#include "missile.h"
#include "../scenes/course.h"
#include "../environment/terrain.h"
#include "../environment/environment.h"
#include "../rendering/color.h"
#include "../rendering/spritesheet.h"
#include "../rendering/animation.h"
#include "../rendering/animations/explosion.h"
#include "../audio/soundeffect.h"
#include "../util/touchinput.h"

#define BALL_RADIUS		4
#define LAUNCH_SPEED		4
#define ROTATE_AMOUNT		(M_PI/40)
#define EXPLOSION_RADIUS	(data->isLarge ? 30 : 20)

static enum {
	WAITING, FLYING_MISSILE, FLYING_BALL
} missileState;

static void reset() {
	missileState = WAITING;
	ProjDefault_Reset();
}

static void launch(float velX, float velY) {
	missileState = FLYING_MISSILE;
	// Set vector length to LAUNCH_SPEED
	float magnitude² = velX * velX + velY * velY;
	velX *= LAUNCH_SPEED / sqrt(magnitude²);
	velY *= LAUNCH_SPEED / sqrt(magnitude²);
	ProjDefault_Launch(velX, velY);
}

static bool pointIsClockwiseToLine(float px, float py, float lx1, float ly1,
		float lx2, float ly2) {
	return (px - lx1)*(ly2 - ly1) - (py - ly1)*(lx2 - lx1) > 0;
}

static void rotateVector(float *vx, float *vy, float angle) {
	float vxNew = (*vx * cos(angle)) - (*vy * sin(angle));
	float vyNew = (*vx * sin(angle)) + (*vy * cos(angle));
	*vx = vxNew, *vy = vyNew;
}

static bool move(float timestep, float *hitX, float *hitY, Terrain_Type *hitType) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	switch (missileState) {
		default:
		case WAITING:
			return false;
		case FLYING_BALL:
			return ProjDefault_Move(timestep, hitX, hitY, hitType);
		case FLYING_MISSILE:
			if (TouchInput_InProgress()) {
				TouchInput_Swipe touch = TouchInput_GetSwipe();
				touch.end.px += Course_GetScreenOffset();
				if (pointIsClockwiseToLine(touch.end.px,
						touch.end.py,
						data->x, data->y,
						data->x + data->velX,
						data->y + data->velY)) {
					rotateVector(&data->velX, &data->velY,
							-ROTATE_AMOUNT*timestep);
				} else {
					rotateVector(&data->velX, &data->velY,
							ROTATE_AMOUNT*timestep);
				}
			}
			// We want all the logic from ProjDefault_Move except
			// gravity, so we save/restore y velocity
			float oldVelY = data->velY;
			bool hitSomething = ProjDefault_Move(timestep, hitX, hitY,
					hitType);
			data->velY = oldVelY;
			return hitSomething;
	}
}

/*
 * Clears a circle of radius EXPLOSION_RADIUS around the ball, sets its state,
 * and plays an explosion animation.
 */
static void doExplosion(float explosionX, float explosionY) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	Env_ClearCircle(explosionX, explosionY, EXPLOSION_RADIUS);
	Animation_Start(animationExplosion,
			Explosion_MakeParams(explosionX, explosionY,
				EXPLOSION_RADIUS + 1),
			NULL);
	SoundEffect_Play(SFX_EXPLOSION, true);
	missileState = FLYING_BALL;
}

static void onHitGround(float hitX, float hitY, Terrain_Type hitType) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	if (missileState == FLYING_MISSILE) {
		doExplosion(data->x, data->y);
	} else {
		ProjDefault_OnHitGround(hitX, hitY, hitType);
	}

	if (missileState == FLYING_BALL
			&& (data->velX*data->velX + data->velY*data->velY > 1)) {
		SoundEffect_Play(SFX_BOUNCE, false);
	}
}

static float getAngleBetween(float x1, float y1, float x2, float y2) {
	// Add M_PI/2 to compensate for sprite being rotated 90° already
	return atan2f(y2 - y1, x2 - x1) + M_PI/2;
}

static void plotTrajectoryPoints(float initX, float initY, float toX, float toY,
		float depth) {
	float nx = toX - initX;
	float ny = toY - initY;
	float nm = sqrt(nx*nx + ny*ny);
	nx *= 5 / nm;
	ny *= 5 / nm;
	for (int t = 5; t <= 20; t += 5) {
		float pointX = initX + t*nx;
		float pointY = initY + t*ny;
		C2D_DrawRectSolid(pointX - 1.5, pointY - 1.5, depth, 3, 3,
				COLOR_DRED);
	}
}

static void draw(float depth) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	TouchInput_Swipe touch = TouchInput_GetSwipe();
	touch.end.px += Course_GetScreenOffset();
	switch (missileState) {
		case WAITING:
			float angle;
			if (TouchInput_InProgress()) {
				angle = getAngleBetween(data->x, data->y,
						touch.end.px, touch.end.py);
				plotTrajectoryPoints(data->x, data->y, touch.end.px,
						touch.end.py, depth);
			} else {
				angle = 0;
			}
			(data->isLarge ? SpriteSheet_DrawCenteredLarge
			               : SpriteSheet_DrawCentered)(
						SPRITE_MISSILE,
						data->x + 1, data->y, depth,
						angle, false, false);
			break;
		case FLYING_MISSILE:
			(data->isLarge ? SpriteSheet_DrawCenteredLarge
			               : SpriteSheet_DrawCentered)(
						SPRITE_MISSILE_FLYING,
						data->x + 1, data->y, depth,
						getAngleBetween(data->x, data->y,
							data->x + data->velX,
							data->y + data->velY),
						false, false);
			break;
		case FLYING_BALL:
			(data->isLarge ? SpriteSheet_DrawCenteredLarge
			               : SpriteSheet_DrawCentered)(
						SPRITE_BALL, data->x + 1,
						data->y + 1, depth, data->rotation,
						false, false);
			break;
	}
}

Projectile projectileMissile = &(struct projectile) {
	.radius =	BALL_RADIUS,
	.reset =	reset,
	.launch =	launch,
	.move =		move,
	.isMoving =	ProjDefault_IsMoving,
	.onHitGround =	onHitGround,
	.draw =		draw
};

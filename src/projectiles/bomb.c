#include <stdlib.h>
#include <math.h>
#include <3ds.h>
#include <citro2d.h>
#include "../projectile.h"
#include "projectile_internal.h"
#include "bomb.h"
#include "../rendering/colors.h"
#include "../rendering/spritesheet.h"
#include "../rendering/animation.h"
#include "../rendering/animations/explosion.h"
#include "../audio/soundeffect.h"
#include "../scenes/course.h"
#include "../environment/terrain.h"
#include "../util/touchinput.h"
#include "../util/macros.h"

#define BALL_RADIUS 4
#define EXPLOSION_RADIUS 20
#define EXPLOSION_BOOST 1
#define MIN_SPEED_AFTER_EXPLOSION 3

#define TIME_SLOW_FACTOR 0.1
#define TIME_SLOW_MAX_FRAMES (60 * 2)

static enum {
	WAITING, FLYING_SHOULD_EXPLODE, FLYING_TIME_SLOWED, FLYING_EXPLODED
} ballState;

static unsigned int timeSlowFrames;
static float rotation, rotationVel;

static void reset() {
	ProjDefault_Reset();
	ballState = WAITING;
	timeSlowFrames = 0;
	rotation = 0;
	rotationVel = 0;
}

static void launch(float velX, float velY) {
	ProjDefault_Launch(velX, velY);
	ballState = FLYING_SHOULD_EXPLODE;
}

static void beginSlowTime() {
	ProjectileI_Data *data = ProjectileI_AccessData();
	data->velX *= TIME_SLOW_FACTOR;
	data->velY *= TIME_SLOW_FACTOR;
}

static void endSlowTime() {
	ProjectileI_Data *data = ProjectileI_AccessData();
	data->velX /= TIME_SLOW_FACTOR;
	data->velY /= TIME_SLOW_FACTOR;
}

/*
 * Sets the velocity to point towards (explosionX, explosionY) and sets its
 * magnitude to max(magnitude + EXPLOSION_BOOST, MIN_SPEED_AFTER_EXPLOSION)
 */
static void boostFromExplosion(float explosionX, float explosionY) {
	ProjectileI_Data *data = ProjectileI_AccessData();

	float relativeX = data->x - explosionX;
	float relativeY = data->y - explosionY;
	float relativeXYLength = sqrt(relativeX*relativeX + relativeY*relativeY);

	float velLength = sqrt(data->velX*data->velX + data->velY*data->velY)
			+ EXPLOSION_BOOST;
	velLength = max(velLength, MIN_SPEED_AFTER_EXPLOSION);

	data->velX = -1 * velLength * (relativeX / relativeXYLength);
	data->velY = -1 * velLength * (relativeY / relativeXYLength);
}

/*
 * Clears a circle of radius EXPLOSION_RADIUS around the ball, sets its state,
 * and plays an explosion animation.
 */
static void doExplosion() {
	ProjectileI_Data *data = ProjectileI_AccessData();
	Terrain_ClearCircle(data->x, data->y, EXPLOSION_RADIUS);
	Animation_Start(animationExplosion,
			Explosion_MakeParams(data->x, data->y,
				EXPLOSION_RADIUS + 1),
			NULL);
	SoundEffect_Play(SFX_EXPLOSION, true);
	if (ballState == FLYING_TIME_SLOWED) endSlowTime();
	ballState = FLYING_EXPLODED;
}

static bool move(float *hitX, float *hitY, Terrain_Type *hitType) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	if (TouchInput_JustStarted() && ballState == FLYING_SHOULD_EXPLODE) {
		beginSlowTime();
		ballState = FLYING_TIME_SLOWED;
	}

	if (TouchInput_JustFinished() && ballState == FLYING_TIME_SLOWED) {
		TouchInput_Swipe touch = TouchInput_GetSwipe();
		doExplosion();
		boostFromExplosion(touch.end.px + Course_GetScreenOffset(),
				touch.end.py);
	}

	float prevVelX = data->velX;
	float prevVelY = data->velY;
	bool hitSomething = ProjDefault_Move(hitX, hitY, hitType);
	if (ballState == FLYING_TIME_SLOWED) {
		// Correct the magnitude any acceleration the ball received
		// Use TIME_SLOW_FACTOR squared because this is acceleration
		data->velX = prevVelX + (data->velX - prevVelX)
				* TIME_SLOW_FACTOR * TIME_SLOW_FACTOR;
		data->velY = prevVelY + (data->velY - prevVelY)
				* TIME_SLOW_FACTOR * TIME_SLOW_FACTOR;

		
		timeSlowFrames++;
		if (timeSlowFrames > TIME_SLOW_MAX_FRAMES) {
			endSlowTime();
			ballState = FLYING_SHOULD_EXPLODE;
		}
	}

	if (ballState == FLYING_EXPLODED) rotation -= rotationVel;

	return hitSomething;
}

static bool isMoving() {
	if (ballState == FLYING_TIME_SLOWED) {
		return true;
	}
	return ProjDefault_IsMoving();
}

static void onHitGround(float hitX, float hitY, Terrain_Type hitType) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	if (ballState == WAITING) return;

	if ((ballState == FLYING_SHOULD_EXPLODE || ballState == FLYING_TIME_SLOWED)
			&& hitX > 0 && hitX < Course_GetFieldWidth()-1
			&& hitY > 0 && hitY < Course_GetFieldHeight()-1) {
		doExplosion();
	}

	if (ballState == FLYING_EXPLODED) {
		// Vector from the center of the ball to the hit position
		float nx = data->x - hitX;
		float ny = data->y - hitY;
		// Amount of the velocity vector in the direction of n
		float p = fabs(nx * data->velY + ny * data->velX)
					/ sqrt(nx*nx + ny*ny);
		rotationVel = p / BALL_RADIUS;

		if (data->velX*data->velX + data->velY*data->velY > 1) {
			SoundEffect_Play(SFX_BOUNCE, false);
		}
	}

	ProjDefault_OnHitGround(hitX, hitY, hitType);
}

static void drawCircle(int x, int y, int radius, u32 color) {
	void plot(int x, int y) {
		C2D_DrawRectSolid(x, y, 0, 1, 1, color);
	}

	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x0 = 0;
	int y0 = radius;

	plot(x, y + radius);
	plot(x, y - radius);
	plot(x + radius, y);
	plot(x - radius, y);

	while(x0 < y0) {
		if(f >= 0) {
			y0--;
			ddF_y += 2;
			f += ddF_y;
		}
		x0++;
		ddF_x += 2;
		f += ddF_x + 1;	
		plot(x + x0, y + y0);
		plot(x - x0, y + y0);
		plot(x + x0, y - y0);
		plot(x - x0, y - y0);
		plot(x + y0, y + x0);
		plot(x - y0, y + x0);
		plot(x + y0, y - x0);
		plot(x - y0, y - x0);
	}
}

static void drawAimingCircle(float depth) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	TouchInput_Swipe swipe = TouchInput_GetSwipe();

	int offsetX = clamp(data->x - 160, 0, Course_GetFieldWidth() - 320);
	float relativeX = data->x - swipe.end.px - offsetX;
	float relativeY = data->y - swipe.end.py;
	float relativeXYLength = sqrt(relativeX*relativeX + relativeY*relativeY);
	float lineX = (EXPLOSION_RADIUS + 5) * (relativeX / relativeXYLength);
	float lineY = (EXPLOSION_RADIUS + 5) * (relativeY / relativeXYLength);

	u32 color = COLOR_DRED;
	if (timeSlowFrames > TIME_SLOW_MAX_FRAMES - 30
			&& (timeSlowFrames / 5) % 2 == 0) {
		color = COLOR_TRANSPARENT;
	}

	drawCircle(data->x, data->y, EXPLOSION_RADIUS, color);
	C2D_DrawLine(data->x - lineX, data->y - lineY, color, data->x + lineX,
			data->y + lineY, color, 1, depth);
}

static void draw(float depth) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	// Adding 1 to x and y when drawing makes it look better
	switch (ballState) {
		case FLYING_TIME_SLOWED:
			drawAimingCircle(depth);
			// fall through
		case WAITING:
		case FLYING_SHOULD_EXPLODE:
			SpriteSheet_DrawCentered(SPRITE_BOMB, data->x + 1,
					data->y + 1, depth, 0, false, false);
			break;
		case FLYING_EXPLODED:
			SpriteSheet_DrawCentered(SPRITE_BALL, data->x + 1,
					data->y + 1, depth, rotation, false, false);
			break;
	}
}

Projectile projectileBomb = &(struct projectile) {
	.radius = 	BALL_RADIUS,
	.reset = 	reset,
	.launch = 	launch,
	.move = 	move,
	.isMoving =	isMoving,
	.onHitGround = 	onHitGround,
	.draw =		draw
};

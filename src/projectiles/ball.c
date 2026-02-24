#include <stdlib.h>
#include <math.h>
#include <3ds.h>
#include <citro2d.h>
#include "../projectile.h"
#include "projectile_internal.h"
#include "ball.h"
#include "../rendering/spritesheet.h"
#include "../scenes/course.h"
#include "../util/touchinput.h"
#include "../util/macros.h"

#define EXPLOSION_RADIUS 20
#define EXPLOSION_BOOST 1
#define MIN_SPEED_AFTER_EXPLOSION 3

#define TIME_SLOW_FACTOR 0.1
#define TIME_SLOW_MAX_FRAMES (60 * 2)

enum {
	WAITING, FLYING_SHOULD_EXPLODE, FLYING_TIME_SLOWED, FLYING_EXPLODED
} ballState;

static unsigned int timeSlowFrames;

static void reset() {
	ProjDefault_Reset();
	ballState = WAITING;
	timeSlowFrames = 0;
}

static void launch(float velX, float velY) {
	ProjDefault_Launch(velX, velY);
	ballState = FLYING_SHOULD_EXPLODE;
}

/*
 * Sets the velocity to point away from (explosionX, explosionY) and sets its
 * magnitude to min(magnitude + EXPLOSION_BOOST, MIN_SPEED_AFTER_EXPLOSION)
 */
static void boostFromExplosion(float explosionX, float explosionY) {
	ProjectileI_Data *data = ProjectileI_AccessData();

	float relativeX = data->x - explosionX;
	float relativeY = data->y - explosionY;
	float relativeXYLength = sqrt(relativeX*relativeX + relativeY*relativeY);

	float velLength = sqrt(data->velX*data->velX + data->velY*data->velY)
			+ EXPLOSION_BOOST;
	if (velLength < 3) velLength = MIN_SPEED_AFTER_EXPLOSION;

	data->velX = velLength * (relativeX / relativeXYLength);
	data->velY = velLength * (relativeY / relativeXYLength);
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

static void move() {
	ProjectileI_Data *data = ProjectileI_AccessData();
	if (TouchInput_JustStarted() && ballState == FLYING_SHOULD_EXPLODE) {
		beginSlowTime();
		ballState = FLYING_TIME_SLOWED;
	}

	if (TouchInput_JustFinished() && ballState == FLYING_TIME_SLOWED) {
		TouchInput_Swipe touch = TouchInput_GetSwipe();
		Course_ClearCircle(data->x, data->y, EXPLOSION_RADIUS);
		endSlowTime();
		int offsetX = clamp(data->x - 160, 0, Course_GetFieldWidth() - 320);
		boostFromExplosion(touch.end.px + offsetX, touch.end.py);
		ballState = FLYING_EXPLODED;
	}

	float prevVelX = data->velX;
	float prevVelY = data->velY;
	ProjDefault_Move();
	if (ballState == FLYING_TIME_SLOWED) {
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
}

static bool isMoving() {
	if (ballState == FLYING_TIME_SLOWED) {
		return true;
	}
	return ProjDefault_IsMoving();
}

static void onHitGround(float hitX, float hitY) {
	if (ballState == WAITING) return;

	ProjectileI_Data *data = ProjectileI_AccessData();

	if ((ballState == FLYING_SHOULD_EXPLODE || ballState == FLYING_TIME_SLOWED)
			&& hitX > 0 && hitX < Course_GetFieldWidth()
			&& hitY > 0 && hitY < Course_GetFieldHeight()) {
		Course_ClearCircle(data->x, data->y, EXPLOSION_RADIUS);
		if (ballState == FLYING_TIME_SLOWED) endSlowTime();
		ballState = FLYING_EXPLODED;
	}

	ProjDefault_OnHitGround(hitX, hitY);
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

	u32 color = C2D_Color32(237, 28, 36, 255);
	if (timeSlowFrames > TIME_SLOW_MAX_FRAMES - 30
			&& (timeSlowFrames / 5) % 2 == 0) {
		color = C2D_Color32(0, 0, 0, 0);
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
			SpriteSheet_Draw(SPRITE_BOMB, data->x + 1, data->y + 1,
					depth, 0);
			break;
		case FLYING_EXPLODED:
			SpriteSheet_Draw(SPRITE_BALL, data->x + 1, data->y + 1,
					depth, 0);
			break;
	}
}

Projectile projectileBall = &(struct projectile) {
	.radius = 	4,
	.reset = 	reset,
	.launch = 	launch,
	.move = 	move,
	.isMoving =	isMoving,
	.onHitGround = 	onHitGround,
	.draw =		draw
};

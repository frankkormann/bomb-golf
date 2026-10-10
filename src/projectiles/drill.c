#include <stdbool.h>
#include <math.h>
#include <3ds.h>
#include <citro2d.h>
#include "../projectile.h"
#include "projectile_internal.h"
#include "drill.h"
#include "missile.h"
#include "../environment/terrain.h"
#include "../environment/environment.h"
#include "../scenes/course.h"
#include "../rendering/color.h"
#include "../rendering/animation.h"
#include "../rendering/animations/explosion.h"
#include "../audio/soundeffect.h"
#include "../util/touchinput.h"
#include "../util/macros.h"

//TODO Figure out how to share these between course and drill
#define LAUNCH_SPEED_MAX		6
#define TOUCH_TO_LAUNCH_VEL_FACTOR	0.05

#define BALL_RADIUS			4
#define EXPLOSION_RADIUS		(data->isLarge ? 30 : 20)
#define EXPLOSION_BOOST			1
#define MIN_SPEED_AFTER_EXPLOSION	3

#define SPIKE_VELOCITY_X		2
#define SPIKE_VELOCITY_Y		4

#define ANIMATION_FRAME_TIME		5

static enum {
	WAITING, FLYING_DRILL, DRILLING, FLYING_BALL
} drillState;

static float timeSlow, animationCounter;

static void reset() {
	drillState = WAITING;
	timeSlow = 0;
	ProjDefault_Reset();
}

static void launch(float velX, float velY) {
	ProjDefault_Launch(velX, velY);
	drillState = FLYING_DRILL;
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
 * Clears a circle of radius EXPLOSION_RADIUS around the drill, sets its state,
 * and plays an explosion animation.
 */
static void doExplosion() {
	ProjectileI_Data *data = ProjectileI_AccessData();
	Env_ClearCircle(data->x, data->y, EXPLOSION_RADIUS);
	Animation_Start(animationExplosion,
			Explosion_MakeParams(data->x, data->y,
				EXPLOSION_RADIUS + 1),
			NULL);
	SoundEffect_Play(SFX_EXPLOSION, true);
	drillState = FLYING_BALL;
}

static void makeLength(float *x, float *y, float len) {
	float mag = sqrt((*x * *x) + (*y * *y));
	*x *= len/mag;
	*y *= len/mag;
}

static void clearPath(int x0, int y0, int x1, int y1, int width) {
	//https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	int dx = x1 - x0 > 0 ? x1 - x0 : x0 - x1;
    	int sx = x0 < x1 ? 1 : -1;
    	int dy = y1 - y0 > 0 ? y0 - y1 : y1 - y0;
    	int sy = y0 < y1 ? 1 : -1;
    	int error = dx + dy;

	while (true) {
		Env_ClearCircle(x0, y0, width);
       		int e2 = 2 * error;
		if (e2 >= dy) {
			if (x0 == x1) break;
			error = error + dy;	
			x0 = x0 + sx;
		}
		if (e2 <= dx) {
			if (y0 == y1) break;
			error = error + dx;
			y0 = y0 + sy;
		}
	}
}

/*
 * Returns true if there is no terran at (x, y) and it's within the course
 */
static bool checkPoint(int x, int y) {
	return x >= 0 && x < Course_GetFieldWidth()
			&& y >= 0 && y < Course_GetFieldHeight()
			&& Terrain_TypeAt(x, y) == TERRAIN_NOTHING;
}

static bool move(float timestep, float *hitX, float *hitY,
			Terrain_Type *hitType) {
	ProjectileI_Data *data = ProjectileI_AccessData();

	if (TouchInput_JustStarted() && drillState == FLYING_DRILL) {
		data->velX = SPIKE_VELOCITY_X * data->velX >= 0 ? 1 : -1;
		data->velY = SPIKE_VELOCITY_Y;
	}

	if (drillState == DRILLING) {
		float vx = data->velX, vy = data->velY,
				rx = -data->velY, ry = data->velX;
		makeLength(&vx, &vy, data->isLarge ? BALL_RADIUS*2 + 12
				: BALL_RADIUS + 6);
		makeLength(&rx, &ry, data->isLarge ? BALL_RADIUS*2 : BALL_RADIUS);
		if (checkPoint(data->x + vx + rx, data->y + vy + ry)
				&& checkPoint(data->x + vx - rx, data->y + vy -ry)) {
			doExplosion();
			boostFromExplosion(data->x + vx, data->y + vy);
		}
		clearPath(data->x, data->y, data->x + data->velX,
				data->y + data->velY,
				data->isLarge ? BALL_RADIUS*2 + 4: BALL_RADIUS + 2);
		animationCounter += timestep;
	}

	float oldVelY = data->velY;
	bool hitSomething = ProjDefault_Move(timestep, hitX, hitY, hitType);
	if (drillState == DRILLING) data->velY = oldVelY - 2*(data->velY - oldVelY);

	return hitSomething;
}

static void onHitGround(float hitX, float hitY, Terrain_Type hitType) {
	if (drillState == FLYING_DRILL) {
		if (hitX > 0 && hitX < Course_GetFieldWidth()-1
				&& hitY > 0 && hitY < Course_GetFieldHeight()-1) {
			drillState = DRILLING;
		} else {
			ProjDefault_OnHitGround(hitX, hitY, hitType);
		}
	}
	if (drillState == DRILLING) {
		if (hitX <= 0 || hitX >= Course_GetFieldWidth()-1
				|| hitY <= 0 || hitY >= Course_GetFieldHeight()-1) {
			ProjDefault_OnHitGround(hitX, hitY, hitType);
		}
	}
	if (drillState == WAITING || drillState == FLYING_BALL) {
		ProjDefault_OnHitGround(hitX, hitY, hitType);
	}
}

//TODO Figure out how to share this between course and drill
static void calculateLaunchVelocity(float *velX, float *velY) {
	TouchInput_Swipe stroke = TouchInput_GetSwipe();
	float projX, projY;
	Projectile_GetPos(&projX, &projY);
	*velX = (float)(stroke.end.px - projX + Course_GetScreenOffset())
			* TOUCH_TO_LAUNCH_VEL_FACTOR;
	*velY = (float)(stroke.end.py - projY) * TOUCH_TO_LAUNCH_VEL_FACTOR;
	float magnitude² = *velX * *velX + *velY * *velY;
	if (magnitude² > LAUNCH_SPEED_MAX*LAUNCH_SPEED_MAX) {
		// Set vector length to LAUNCH_SPEED_MAX
		*velX *= LAUNCH_SPEED_MAX / sqrt(magnitude²);
		*velY *= LAUNCH_SPEED_MAX / sqrt(magnitude²);
	}
}

static void plotTrajectoryPoint(float initX, float initY, float velX, float velY,
		int framesInFuture, float size, float depth, u32 color) {
	float pointX = initX + (velX * framesInFuture);
	float pointY = initY + (velY * framesInFuture)
			+ (0.5 * PROJECTILE_GRAVITY * framesInFuture*framesInFuture);
	C2D_DrawRectSolid(pointX - size/2, pointY - size/2, depth, size, size,
			color);
}

static void plotTrajectoryPoints(float initX, float initY, float depth) {
	float velX, velY;
	calculateLaunchVelocity(&velX, &velY);

	float strength = (velX*velX + velY*velY)
			/ (LAUNCH_SPEED_MAX*LAUNCH_SPEED_MAX);
	u32 color = strength > 0.75 ? COLOR_DRED
			: strength > 0.5 ? COLOR_RED
			: strength > 0.25 ? COLOR_ORANGE
			: COLOR_LGREEN;
	for (int t = 5; t <= 20; t += 5) {
		plotTrajectoryPoint(initX, initY, velX, velY, t, 3, 1, color);
	}
}

static void draw(float depth) {
	ProjectileI_Data *data = ProjectileI_AccessData();
	// Adding 1 to x and y when drawing makes it look better
	switch (drillState) {
		case WAITING:
			if (TouchInput_InProgress()) {
				plotTrajectoryPoints(data->x, data->y,
						nextafterf(depth, -1));
			}
			(data->isLarge ? SpriteSheet_DrawCenteredLarge
			               : SpriteSheet_DrawCentered)(
						SPRITE_DRILL, data->x + 1,
						data->y + 1, depth, 0, false, false);
			break;
		case DRILLING:
		case FLYING_DRILL:
			float angle = atan2f(data->velY, data->velX) + M_PI/2;
			bool flip = ((int)animationCounter / ANIMATION_FRAME_TIME)
					% 2 == 0;
			(data->isLarge ? SpriteSheet_DrawCenteredLarge
			               : SpriteSheet_DrawCentered)(
						SPRITE_DRILL, data->x + 1,
						data->y + 1, depth,
						angle, flip, false);
			break;
		case FLYING_BALL:
			(data->isLarge ? SpriteSheet_DrawCenteredLarge
			               : SpriteSheet_DrawCentered)(
						SPRITE_BALL, data->x + 1,
						data->y + 1, depth,
						data->rotation, false, false);
			break;
	}
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

#include <stdlib.h>
#include <math.h>
#include <3ds.h>
#include <citro2d.h>
#include "projectile.h"
#include "projectiles/projectile_internal.h"
#include "scenes/course.h"
#include "rendering/spritesheet.h"
#include "util/macros.h"
#include "levelio.h"

#define LAST_POS_COUNT 20
#define STOPPED_THRESHOLD 1.5

#define GRAVITY 0.1
#define BOUNCE_VELOCITY_RETENTION_X 0.8
#define BOUNCE_VELOCITY_RETENTION_Y 0.6

static Projectile proj;
static ProjectileI_Data data;

static bool oldIsMoving;

static size_t lastPosIndex;
static float lastXs[LAST_POS_COUNT];
static float lastYs[LAST_POS_COUNT];

void Projectile_SetType(Projectile newProj) {
	proj = newProj;
	Projectile_Reset();
}

void Projectile_SetPos(float x, float y) {
	data.x = x;
	data.y = y;
}

void Projectile_GetPos(float *x, float *y) {
	*x = data.x;
	*y = data.y;
}

bool Projectile_IsMoving() {
	return proj->isMoving();
}

void Projectile_Reset() {
	proj->reset();
	for (size_t i = 0; i < LAST_POS_COUNT; i++) {
		lastXs[i] = data.x;
		lastYs[i] = data.y;
	}
}

void Projectile_Launch(float velX, float velY) {
	proj->launch(velX, velY);
}

void Projectile_Update() {
	oldIsMoving = Projectile_IsMoving();
	proj->move();

	lastXs[lastPosIndex] = data.x;
	lastYs[lastPosIndex] = data.y;
	lastPosIndex = (lastPosIndex + 1) % LAST_POS_COUNT;

	if (oldIsMoving && !Projectile_IsMoving()) {
		Projectile_Reset();
	}
}

void Projectile_Draw(float depth) {
	proj->draw(depth);
}

void Projectile_CenterViewC2D(gfxScreen_t screen) {
	int screenWidth = screen == GFX_TOP ? 400 : 320;
	int offset = clamp(data.x - screenWidth / 2, 0,
			Course_GetFieldWidth() - screenWidth);
	C2D_ViewTranslate(-offset, 0);
}

ProjectileI_Data* ProjectileI_AccessData() {
	return &data;
}

void ProjDefault_Reset() {
	data.velX = 0;
	data.velY = 0;
}

void ProjDefault_Launch(float velX, float velY) {
	data.velX = velX;
	data.velY = velY;
}

static void checkCircle(int x, int y, int radius, int *hitX, int *hitY) {
	int hitsX = 0, hitsY = 0;
	int numHits = 0;
	void checkTerrain(int x, int y) {
		if (Course_CheckTerrain(x, y)) {
			hitsX += x;
			hitsY += y;
			numHits++;
		}
	}

	int f = 1 - radius;
	int ddF_x = 0;
	int ddF_y = -2 * radius;
	int x0 = 0;
	int y0 = radius;

	checkTerrain(x, y + radius);
	checkTerrain(x, y - radius);
	checkTerrain(x + radius, y);
	checkTerrain(x - radius, y);

	while(x0 < y0) {
		if(f >= 0) {
			y0--;
			ddF_y += 2;
			f += ddF_y;
		}
		x0++;
		ddF_x += 2;
		f += ddF_x + 1;	
		checkTerrain(x + x0, y + y0);
		checkTerrain(x - x0, y + y0);
		checkTerrain(x + x0, y - y0);
		checkTerrain(x - x0, y - y0);
		checkTerrain(x + y0, y + x0);
		checkTerrain(x - y0, y + x0);
		checkTerrain(x + y0, y - x0);
		checkTerrain(x - y0, y - x0);
	}

	if (numHits == 0) {
		*hitX = 0;
		*hitY = 0;
	} else {
		*hitX = hitsX / numHits;
		*hitY = hitsY / numHits;
	}
}

static void raycast(int x0, int y0, int x1, int y1, bool *hitSomething,
		int *ultimateX, int *ultimateY, int *penultimateX,
		int *penultimateY, int *hitX, int *hitY) {
	//https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
	int dx = x1 - x0 > 0 ? x1 - x0 : x0 - x1;
    	int sx = x0 < x1 ? 1 : -1;
    	int dy = y1 - y0 > 0 ? y0 - y1 : y1 - y0;
    	int sy = y0 < y1 ? 1 : -1;
    	int error = dx + dy;

	*hitSomething = false;

	*penultimateX = x0;
	*penultimateY = y0;
	while (true) {
		*ultimateX = x0;
		*ultimateY = y0;

		checkCircle(x0, y0, proj->radius, hitX, hitY);
		if (*hitX != 0 || *hitY != 0) {
			*hitSomething = true;
			break;
		}
		*penultimateX = x0;
		*penultimateY = y0;

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

void ProjDefault_Move() {
	data.velY += GRAVITY;

	bool hasHitSomething;
	int finalX, finalY, lastOkX, lastOkY, hitX, hitY;
	raycast(roundf(data.x), roundf(data.y), roundf(data.x + data.velX),
			roundf(data.y + data.velY), &hasHitSomething, &finalX,
			&finalY, &lastOkX, &lastOkY, &hitX, &hitY);

	if (hasHitSomething) {
		data.x = lastOkX;
		data.y = lastOkY;
		proj->onHitGround(hitX, hitY);
	} else {
		data.x += data.velX;
		data.y += data.velY;
	}
}

bool ProjDefault_IsMoving() {
	float minX = LEVEL_MAX_WIDTH, minY = LEVEL_HEIGHT, maxX = 0, maxY = 0;
	for (size_t i = 0; i < LAST_POS_COUNT; i++) {
		if (minX > lastXs[i]) minX = lastXs[i];
		if (minY > lastYs[i]) minY = lastYs[i];
		if (maxX < lastXs[i]) maxX = lastXs[i];
		if (maxY < lastYs[i]) maxY = lastYs[i]; 
	}
	return maxX - minX > STOPPED_THRESHOLD || maxY - minY > STOPPED_THRESHOLD;
}

void ProjDefault_OnHitGround(float hitX, float hitY) {
	// Use this formula from https://math.stackexchange.com/a/13263 to reflect
	// the velocity vector over the line between the hit point and the center
	// of the ball:
	//	v' = v - 2(v·n)n
	// where v' is the new velocity, v is the current velocity, and n is the
	// normalized vector to reflect across. Also reverses the direction of v'

	float nX = data.x - hitX;
	float nY = data.y - hitY;
	float lenN = sqrt(nX*nX + nY*nY);
	nX /= lenN;
	nY /= lenN;

	float vDotN = (nX * data.velX) + (nY * data.velY);
	data.velX -= 2 * vDotN * nX;
	data.velY -= 2 * vDotN * nY;

	data.velX *= BOUNCE_VELOCITY_RETENTION_X;
	data.velY *= BOUNCE_VELOCITY_RETENTION_Y;
}

void ProjDefault_Draw(float depth) {}


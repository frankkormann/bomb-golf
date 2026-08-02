#include <malloc.h>
#include <stdbool.h>
#include <math.h>
#include <citro2d.h>
#include "obstacle.h"
#include "../rendering/spritesheet.h"
#include "../rendering/animation.h"
#include "../rendering/animations/smoke.h"
#include "../util/list.h"
#include "../util/macros.h"

typedef struct {
	SpriteSheet_ObstSprite spr1;
	SpriteSheet_ObstSprite spr2;
	bool flipHoriz;
	bool flipVert;
	bool rotate;
	int *xs;
	int *ys;
	int numPoints;
	int curPoint;
	int width;
	int height;
	float speed;
	int pathCounter;
	int animCounter;
} Obstacle;

static List obstacleList;

bool Obstacle_Init() {
	obstacleList = List_Create();
	return obstacleList == NULL;
}

// Signature designed for List operations
static void freeObstacle(void *elem) {
	Obstacle *obst = (Obstacle*)elem;
	free(obst->xs);
	free(obst->ys);
	free(obst);
}

void Obstacle_Exit() {
	List_ForEach(obstacleList, freeObstacle);
	List_Free(obstacleList);
}

static void setFlipAndRotation(Obstacle *obst) {
	int curX = obst->xs[obst->curPoint];
	int curY = obst->ys[obst->curPoint];
	int nextX = obst->xs[(obst->curPoint + 1) % obst->numPoints];
	int nextY = obst->ys[(obst->curPoint + 1) % obst->numPoints];

	if (nextX > curX) {
		obst->flipHoriz = false;
		obst->flipVert  = false;
		obst->rotate    = false;
	} else if (nextX < curX) {
		obst->flipHoriz = true;
		obst->flipVert  = false;
		obst->rotate    = false;
	} else if (nextY > curY) {
		obst->flipHoriz = false;
		obst->flipVert  = true;
		obst->rotate    = true;
	} else if (nextY < curY) {
		obst->flipHoriz = true;
		obst->flipVert  = true;
		obst->rotate    = true;
	} else {
		obst->flipHoriz = false;
		obst->flipVert  = false;
		obst->rotate    = false;
	}
}

bool Obstacle_Add(Obstacle_Data data) {
	Obstacle *obst = malloc(sizeof(*obst));
	if (!obst) goto f_obst;

	obst->xs = malloc(sizeof(*obst->xs) * data.numPoints);
	if (!obst->xs) goto f_xs;

	obst->ys = malloc(sizeof(*obst->ys) * data.numPoints);
	if (!obst->ys) goto f_ys;

	C2D_Image sprImg = SpriteSheet_GetObstacleImage(data.sprite1);
	obst->width = sprImg.subtex->width;
	obst->height = sprImg.subtex->height;

	obst->spr1 = data.sprite1;
	obst->spr2 = data.sprite2;
	obst->flipHoriz = data.numPoints > 1 && data.xs[0] > data.xs[1];
	obst->flipVert = data.numPoints > 1 && data.xs[0] == data.xs[1]
			&& data.ys[0] > data.ys[1];
	for (int i = 0; i < data.numPoints; i++) {
		obst->xs[i] = data.xs[i];
		obst->ys[i] = data.ys[i];
	}
	obst->numPoints = data.numPoints;
	obst->curPoint = 0;
	obst->speed = data.speed;
	obst->pathCounter = 0;
	obst->animCounter = 0;  //TODO Consider randomizing this

	setFlipAndRotation(obst);

	if (!List_Push(obstacleList, obst)) goto f_List_Push;

	return true;

f_List_Push:
	free(obst->ys);
f_ys:
	free(obst->xs);
f_xs:
	free(obst);
f_obst:
	return false;
}

static void getObstaclePos(Obstacle *obst, float *x, float *y) {
	// x(t) = x_0 - t(v(x_0 - x_1) / sqrt((x_0 - x_1)^2 + (y_0 - y_1)^2))
	// y(t) = y_0 - t(v(y_0 - y_1) / sqrt((x_0 - x_1)^2 + (y_0 - y_1)^2))

	int curX = obst->xs[obst->curPoint];
	int curY = obst->ys[obst->curPoint];
	int nextX = obst->xs[(obst->curPoint + 1) % obst->numPoints];
	int nextY = obst->ys[(obst->curPoint + 1) % obst->numPoints];

	int dx = curX - nextX, dy = curY - nextY;
	float radical = sqrt(dx*dx + dy*dy);

	if (radical == 0) {
		*x = curX;
		*y = curY;
	} else {
		*x = curX - (obst->pathCounter * obst->speed * dx) / radical;
		*y = curY - (obst->pathCounter * obst->speed * dy) / radical;
	}
}

//FIXME Improve hitboxes
static bool obstacleIntersects(Obstacle *obst, int x, int y) {
	float ox, oy;
	getObstaclePos(obst, &ox, &oy);
	return x >= ox - obst->width/2
			&& x <= ox + obst->width/2
			&& y >= oy - obst->height/2
			&& y <= oy + obst->height/2;
}

// Frees an obstacle and plays smoke animation
// Signature designed for List operations
static void destroyObstacle(void *elem) {
	Obstacle *obst = (Obstacle*)elem;
	float x, y;
	getObstaclePos(obst, &x, &y);
	Animation_Start(animationSmoke, Smoke_MakeParams(x, y), NULL);
	freeObstacle(obst);
}

void Obstacle_Destroy(int argX, int argY) {
	// Need these to be static so 3ds doesn't crash when they're used in
	// intersects
	static int x, y;
	x = argX, y = argY;
	bool intersects(void *elem) {
		return obstacleIntersects((Obstacle*)elem, x, y);
	}
	List_Filter(obstacleList, intersects, destroyObstacle);
}

void Obstacle_DestroyCircle(int argX, int argY, int argRadius) {
	// Need these to be static so 3ds doesn't crash when they're used in
	// intersects
	static int x, y, radius;
	//https://stackoverflow.com/a/1879223
	x = argX, y = argY, radius = argRadius;
	bool intersects(void *elem) {
		Obstacle *obst = (Obstacle*)elem;
		float ox, oy;
		getObstaclePos(obst, &ox, &oy);

		float closestX = clamp(x, ox, ox + obst->width);
		float closestY = clamp(y, oy, oy + obst->height);
		float distX = x - closestX;
		float distY = y - closestY;

		return distX*distX + distY*distY <= radius*radius;
	}
	List_Filter(obstacleList, intersects, destroyObstacle);
}

void Obstacle_Clear() {
	bool returnTrue() { return true; };
	List_Filter(obstacleList, returnTrue, freeObstacle);
}

bool Obstacle_IsAt(int argX, int argY) {
	// Need these to be static so 3ds doesn't crash when they're used in
	// intersects
	static int x, y;
	x = argX, y = argY;
	bool intersects(void *elem) {
		return obstacleIntersects((Obstacle*)elem, x, y);
	}
	return List_Check(obstacleList, intersects);
}

void Obstacle_Update() {
	void update(void *elem) {
		Obstacle *obst = (Obstacle*)elem;
		obst->pathCounter++;
		obst->animCounter++;

		float x, y;
		getObstaclePos(obst, &x, &y);

		int curX = obst->xs[obst->curPoint];
		int curY = obst->ys[obst->curPoint];
		int nextX = obst->xs[(obst->curPoint + 1) % obst->numPoints];
		int nextY = obst->ys[(obst->curPoint + 1) % obst->numPoints];

		if ((nextX > curX && x >= nextX) || (nextX < curX && x <= nextX)
				|| (nextY > curY && y >= nextY)
				|| (nextY < curY && y <= nextY)) {
			obst->pathCounter = 0;
			obst->curPoint = (obst->curPoint + 1) % obst->numPoints;
			setFlipAndRotation(obst);
		}
	}
	List_ForEach(obstacleList, update);
}

void Obstacle_Draw(float argDepth) {
	// Need this to be static so 3ds doesn't crash when it's used in draw
	static float depth;
	depth = argDepth;
	void draw(void *elem) {
		Obstacle *obst = (Obstacle*)elem;
		float x, y;
		getObstaclePos(obst, &x, &y);
		SpriteSheet_DrawObstacle(
				(obst->animCounter / 30) % 2 == 0
					? obst->spr1 : obst->spr2,
				x, y, depth,
				obst->rotate ? M_PI/2 : 0,
				obst->flipHoriz, obst->flipVert
			);
	}
	List_ForEach(obstacleList, draw);
}

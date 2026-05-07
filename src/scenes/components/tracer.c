#include <malloc.h>
#include <stdbool.h>
#include <citro2d.h>
#include "tracer.h"
#include "../../rendering/colors.h"
#include "../../util/list.h"

#define POINT_SIZE 1.2

struct tracer {
	List renderList;
	float maxX;
	float maxY;
};

typedef struct {
	float x;
	float y;
} Point;

// Assigned and used in Tracer_Draw + helper function
static float drawDepth;
static float drawAddX, drawAddY;
static float drawMultX, drawMultY;

Tracer Tracer_Create(float maxX, float maxY) {
	Tracer tracer = malloc(sizeof(*tracer));
	if (!tracer) return NULL;

	tracer->renderList = List_Create();
	if (!tracer->renderList) {
		free(tracer);
		return NULL;
	}
	tracer->maxX = maxX;
	tracer->maxY = maxY;

	return tracer;
}

void Tracer_Free(Tracer tracer) {
	List_ForEach(tracer->renderList, free);
	List_Free(tracer->renderList);
	free(tracer);
}

bool Tracer_AddPoint(Tracer tracer, float x, float y) {
	Point *new = malloc(sizeof(*new));
	if (!new) return false;
	new->x = x;
	new->y = y;

	Point *last = List_Last(tracer->renderList);
	if (last && last->x == new->x && last->y == new->y) return true;

	return List_Append(tracer->renderList, new);
}

// 3DS doesn't like it when this is defined as a nested function
static void drawElem(void *elem) {
	Point *p = elem;
	C2D_DrawRectSolid(
			(p->x * drawMultX) + drawAddX,
			(p->y * drawMultY) + drawAddY,
			drawDepth,
			POINT_SIZE, POINT_SIZE, COLOR_DRED
		);
}

void Tracer_Draw(Tracer tracer, float x, float y, float depth, float width,
		float height) {
	drawDepth = depth;
	drawAddX = x;
	drawAddY = y;
	drawMultX = width / tracer->maxX;
	drawMultY = height / tracer->maxY;
	List_ForEach(tracer->renderList, drawElem);
}

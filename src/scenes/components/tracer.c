#include <malloc.h>
#include <stdbool.h>
#include <citro2d.h>
#include "tracer.h"
#include "../../rendering/colors.h"
#include "../../util/queue.h"

#define MAX_TO_DRAW_PER_FRAME (C2D_DEFAULT_MAX_OBJECTS - 200)
#define POINT_SIZE 4

struct tracer {
	C3D_Tex tex;
	Tex3DS_SubTexture subtex;
	C3D_RenderTarget *texTarget;
	bool isDirty;
	Queue renderQueue;
	float maxX;
	float maxY;
};

typedef struct {
	float x;
	float y;
} Point;

Tracer Tracer_Create(float maxX, float maxY) {
	Tracer tracer = malloc(sizeof(*tracer));
	if (!tracer) goto f_tracer;

	// Tex dimensions must be a power of 2
	int texWidth = __builtin_stdc_bit_ceil((unsigned int)maxX);
	int texHeight = __builtin_stdc_bit_ceil((unsigned int)maxY);
	if (!C3D_TexInitVRAM(&tracer->tex, texWidth, texHeight, GPU_RGBA8)) goto f_tex;

	tracer->subtex = (Tex3DS_SubTexture) {
		.width	= maxX,
		.height = maxY,
		.left	= 0,
		.right	= (float)maxX/ texWidth,
		.top	= 1,
		.bottom	= 1 - ((float)maxY / texHeight)
	};
	tracer->texTarget = C3D_RenderTargetCreateFromTex(&tracer->tex,
			GPU_TEXFACE_2D, 0, -1);
	if (!tracer->texTarget) goto f_texTarget;

	tracer->renderQueue = Queue_Create();
	if (!tracer->renderQueue) goto f_renderQueue;

	tracer->isDirty = true;
	tracer->maxX = maxX;
	tracer->maxY = maxY;

	return tracer;

f_renderQueue:
	C3D_RenderTargetDelete(tracer->texTarget);
f_texTarget:
	C3D_TexDelete(&tracer->tex);
f_tex:
	free(tracer);
f_tracer:
	return NULL;
}

void Tracer_Free(Tracer tracer) {
	C3D_TexDelete(&tracer->tex);
	C3D_RenderTargetDelete(tracer->texTarget);
	while (!Queue_IsEmpty(tracer->renderQueue)) {
		free(Queue_Pop(tracer->renderQueue));
	}
	Queue_Free(tracer->renderQueue);
	free(tracer);
}

bool Tracer_AddPoint(Tracer tracer, float x, float y) {
	Point *p = malloc(sizeof(*p));
	if (!p) return false;
	p->x = x;
	p->y = y;

	if (!Queue_Push(tracer->renderQueue, p)) {
		free(p);
		return false;
	}
	return true;
}

void Tracer_UpdateGraphics(Tracer tracer) {
	if (tracer->isDirty) {
		C2D_TargetClear(tracer->texTarget, COLOR_TRANSPARENT);
		tracer->isDirty = false;
	}
	unsigned int thingsDrawnThisFrame = 0;
	C2D_SceneBegin(tracer->texTarget);

	while (thingsDrawnThisFrame < MAX_TO_DRAW_PER_FRAME
			&& !Queue_IsEmpty(tracer->renderQueue)) {
		Point *p = Queue_Pop(tracer->renderQueue);
		C2D_DrawRectSolid(p->x, p->y, 0, POINT_SIZE, POINT_SIZE,
				COLOR_DRED);
		free(p);

		thingsDrawnThisFrame++;
	}
}

void Tracer_Draw(Tracer tracer, int x, int y, float depth, int width, int height) {
	C2D_Image img = { &tracer->tex, &tracer->subtex };
	C3D_TexSetFilter(&tracer->tex, GPU_LINEAR, GPU_LINEAR);
	C2D_DrawImageAt(img, x, y, depth, NULL, width / tracer->maxX,
			height / tracer->maxY);
}

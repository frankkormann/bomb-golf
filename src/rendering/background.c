#include <citro2d.h>
#include <malloc.h>
#include <stdbool.h>
#include "background.h"
#include "../util/queue.h"
#include "../rendering/spritesheet.h"

#define MAX_TO_DRAW_PER_FRAME (C2D_DEFAULT_MAX_OBJECTS - 200)

typedef struct {
	int x;
	int y;
	u32 color;
} RenderPoint;

typedef struct {
	int x;
	int y;
	BG_Tile type;
} RenderTile;

typedef struct {
	enum { OBJ_TILE, OBJ_POINT } type;
	union {
		RenderPoint point;
		RenderTile tile;
	};
} RenderObj;

struct background {
	C3D_Tex tex;
	Tex3DS_SubTexture subtex;
	C3D_RenderTarget *texTarget;
	Queue renderQueue;
	u32 clearColor;
	bool isDirty;
};

Background BG_Create(unsigned int width, unsigned int height, u32 clearColor) {
	Background bg = malloc(sizeof(*bg));
	if (!bg) goto failed;

	// Tex dimensions must be a power of 2
	int texWidth = __builtin_stdc_bit_ceil(width);
	int texHeight = __builtin_stdc_bit_ceil(height);

	bool success = C3D_TexInitVRAM(&bg->tex, texWidth, texHeight, GPU_RGBA8);
	if (!success) goto failed;

	bg->subtex = (Tex3DS_SubTexture) {
		.width	= width,
		.height = height,
		.left	= 0,
		.right	= (float)width/ texWidth,
		.top	= 1,
		.bottom	= 1 - ((float)height / texHeight)
	};
	bg->texTarget = C3D_RenderTargetCreateFromTex(&bg->tex, GPU_TEXFACE_2D, 0,
					GPU_RB_DEPTH24);
	if (!bg->texTarget) goto failed;

	bg->renderQueue = Queue_Create();
	if (!bg->renderQueue) goto failed;

	bg->clearColor = clearColor;
	bg->isDirty = true;

	return bg;

failed:
	BG_Free(bg);
	return NULL;
}

void BG_Free(Background bg) {
	if (bg) {
		C3D_TexDelete(&bg->tex);
		if (bg->texTarget) C3D_RenderTargetDelete(bg->texTarget);
		if (bg->renderQueue) Queue_Free(bg->renderQueue);
		free(bg);
	}
}

bool BG_DrawTile(Background bg, BG_Tile tile, int x, int y, bool clearPrevious) {
	if (clearPrevious) {
		if (!BG_DrawTile(bg, TILE_CLEAR, x, y, false)) return false;
	}

	RenderObj *o = malloc(sizeof(*o));
	if (!o) return false;

	o->type = OBJ_TILE;
	o->tile = (RenderTile) { .x = x, .y = y, .type = tile };
	if (!Queue_Push(bg->renderQueue, o)) {
		free(o);
		return false;
	}
	return true;
}

bool BG_ClearPixel(Background bg, int x, int y) {
	RenderObj *o = malloc(sizeof(*o));
	if (!o) return false;

	o->type = OBJ_POINT;
	o->point = (RenderPoint) { .x = x, .y = y, .color = bg->clearColor };
	if (!Queue_Push(bg->renderQueue, o)) {
		free(o);
		return false;
	}
	return true;
}

void drawPoint(RenderPoint point) {
	C2D_DrawRectSolid(point.x, point.y, 0, 1, 1, point.color);
}

void drawTile(RenderTile tile, u32 backgroundColor) {
	float depth = 0;
	// Sprites are drawn at their center
	float spriteX = tile.x + BG_TILE_SIZE / 2;
	float spriteY = tile.y + BG_TILE_SIZE / 2;
	switch (tile.type) {
		case TILE_CLEAR:
			C2D_ImageTint tint;
			C2D_PlainImageTint(&tint, backgroundColor, 1);
			SpriteSheet_Draw(SPRITE_BLOCK, spriteX, spriteY, depth, 0,
					false, false, &tint);
			break;
		case TILE_GRASS_TOP:
			SpriteSheet_Draw(SPRITE_GRASS, spriteX, spriteY,
					depth, 0, false, false, NULL);
			break;
		case TILE_DIRT_TOP_N:
			SpriteSheet_Draw(SPRITE_DIRT, spriteX, spriteY,
					depth, 0, false, false, NULL);
			break;
		case TILE_DIRT_TOP_S:
			SpriteSheet_Draw(SPRITE_DIRT, spriteX, spriteY,
					depth, 0, false, true, NULL);
			break;
		case TILE_DIRT_TOP_W:
			SpriteSheet_Draw(SPRITE_DIRT, spriteX, spriteY,
					depth, M_PI / 2, false, false, NULL);
			break;
		case TILE_DIRT_TOP_E:
			SpriteSheet_Draw(SPRITE_DIRT, spriteX, spriteY,
					depth, M_PI / 2, false, true, NULL);
			break;
		case TILE_DIRT_INTERNAL:
			SpriteSheet_Draw(SPRITE_DIRT_INTERNAL, spriteX, spriteY,
					depth, 0, false, false, NULL);
			break;
		case TILE_DIRT_TRI_NW:
			SpriteSheet_Draw(SPRITE_DIRT_TRIANGLE, spriteX, spriteY,
					depth, M_PI, false, false, NULL);
			break;
		case TILE_DIRT_TRI_NE:
			SpriteSheet_Draw(SPRITE_DIRT_TRIANGLE, spriteX, spriteY,
					depth, M_PI, true, false, NULL);
			break;
		case TILE_DIRT_TRI_SW:
			SpriteSheet_Draw(SPRITE_DIRT_TRIANGLE, spriteX, spriteY,
					depth, 0, true, false, NULL);
			break;
		case TILE_DIRT_TRI_SE:
			SpriteSheet_Draw(SPRITE_DIRT_TRIANGLE, spriteX, spriteY,
					depth, 0, false, false, NULL);
			break;
		case TILE_DIRT_TRI_FILL_NW:
			SpriteSheet_Draw(SPRITE_DIRT_TRIANGLE_FILLER, spriteX,
					spriteY, depth, 0, true, false, NULL);
			break;
		case TILE_DIRT_TRI_FILL_NE:
			SpriteSheet_Draw(SPRITE_DIRT_TRIANGLE_FILLER, spriteX,
					spriteY, depth, 0, false, false, NULL);
			break;
		case TILE_DIRT_TRI_FILL_SW:
			SpriteSheet_Draw(SPRITE_DIRT_TRIANGLE_FILLER, spriteX,
					spriteY, depth, 0, true, true, NULL);
			break;
		case TILE_DIRT_TRI_FILL_SE:
			SpriteSheet_Draw(SPRITE_DIRT_TRIANGLE_FILLER, spriteX,
					spriteY, depth, 0, false, true, NULL);
			break;
		case TILE_GRASS_TRI_NW:
			SpriteSheet_Draw(SPRITE_GRASS_TRIANGLE_2, spriteX, spriteY,
					depth, M_PI, false, false, NULL);
			break;
		case TILE_GRASS_TRI_NE:
			SpriteSheet_Draw(SPRITE_GRASS_TRIANGLE_2, spriteX, spriteY,
					depth, M_PI, true, false, NULL);
			break;
		case TILE_GRASS_TRI_SW:
			SpriteSheet_Draw(SPRITE_GRASS_TRIANGLE_1, spriteX, spriteY,
					depth, 0, true, false, NULL);
			break;
		case TILE_GRASS_TRI_SE:
			SpriteSheet_Draw(SPRITE_GRASS_TRIANGLE_1, spriteX, spriteY,
					depth, 0, false, false, NULL);
			break;
		case TILE_GRASS_HALF_N:
			SpriteSheet_Draw(SPRITE_GRASS_HALF_HORIZ, spriteX, spriteY,
					depth, 0, false, false, NULL);
			break;
		case TILE_GRASS_HALF_S:
			SpriteSheet_Draw(SPRITE_GRASS_HALF_HORIZ, spriteX, spriteY,
					depth, 0, false, true, NULL);
			break;
		case TILE_GRASS_HALF_W:
			SpriteSheet_Draw(SPRITE_GRASS_HALF_VERT, spriteX, spriteY,
					depth, 0, false, false, NULL);
			break;
		case TILE_GRASS_HALF_E:
			SpriteSheet_Draw(SPRITE_GRASS_HALF_VERT, spriteX, spriteY,
					depth, 0, false, true, NULL);
			break;
		case TILE_GRASS_TRI_FILL_W:
			SpriteSheet_Draw(SPRITE_GRASS_TRIANGLE_FILLER, spriteX,
					spriteY, depth, 0, true, false, NULL);
			break;
		case TILE_GRASS_TRI_FILL_E:
			SpriteSheet_Draw(SPRITE_GRASS_TRIANGLE_FILLER, spriteX,
					spriteY, depth, 0, false, false, NULL);
			break;
		case NUM_TILES:
			SpriteSheet_Draw(SPRITE_BLOCK, spriteX, spriteY, depth, 0,
					false, false, NULL);
			break;
	}
}

void BG_ClearAll(Background bg) {
	bg->isDirty = true;
}

void BG_UpdateGraphics(Background bg) {
	if (!BG_IsUpdating(bg)) return;

	if (bg->isDirty) {
		C2D_TargetClear(bg->texTarget, bg->clearColor);
		bg->isDirty = false;
	}

	unsigned int thingsDrawnThisFrame = 0;
	C2D_SceneBegin(bg->texTarget);

	while (thingsDrawnThisFrame < MAX_TO_DRAW_PER_FRAME
			&& !Queue_IsEmpty(bg->renderQueue)) {
		RenderObj *o = Queue_FastPop(bg->renderQueue);
		switch (o->type) {
			case OBJ_POINT:
				drawPoint(o->point);
				break;
			case OBJ_TILE:
				drawTile(o->tile, bg->clearColor);
				break;
		}
		free(o);

		thingsDrawnThisFrame++;
	}

	// Because we used Queue_FastPop
	Queue_Prune(bg->renderQueue);
}

bool BG_IsUpdating(Background bg) {
	return !Queue_IsEmpty(bg->renderQueue) || bg->isDirty;
}

void BG_Draw(Background bg, float x, float y, float depth, float scaleX,
		float scaleY) {
	C2D_Image img = { &bg->tex, &bg->subtex };
	C2D_DrawImageAt(img, x, y, depth, NULL, scaleX, scaleY);
}

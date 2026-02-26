#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "editor.h"
#include "title.h"
#include "../rendering/rendertarget.h"
#include "../rendering/background.h"
#include "../projectiles/ball.h"
#include "../util/touchinput.h"
#include "../util/macros.h"
#include "../levelio.h"

#define SKY_COLOR (C2D_Color32(101, 101, 255, 255))
#define TERRAIN_COLOR (C2D_Color32(0, 255, 0, 255))
#define RED_COLOR (C2D_Color32(136, 0, 21, 255))

#define HOLE_WIDTH (BG_TILE_SIZE * 2)
#define HOLE_HEIGHT (BG_TILE_SIZE * 4)

#define SCROLL_UNIT BG_TILE_SIZE

static Background bg;
static float scroll;

static Background tileSelectorBG;
static BG_Tile (*tiles)[LEVEL_HEIGHT_TILES];
static BG_Tile selectedTile;

static int holeX, holeY;
static int projX, projY;

static unsigned int level;

Scene_Params Editor_MakeParams(unsigned int level) {
	return (Scene_Params) { .editor = {
		.level = level
	} };
}

static bool sceneInit(Scene_Params params) {
	bg = BG_Create(LEVEL_MAX_WIDTH, LEVEL_HEIGHT, SKY_COLOR);
	if (!bg) goto failed;

	tileSelectorBG = BG_Create((BG_TILE_SIZE + 2) * NUM_TILES - 2, BG_TILE_SIZE,
			SKY_COLOR);
	if (!tileSelectorBG) goto failed;

	char path[20];
	sprintf(path, "sdmc:/level_%i.bin", params.editor.level);
	LevelIO_Hole hole;
	LevelIO_Proj proj;
	int width;
	if (LevelIO_Read(path, &hole, &proj, &tiles, &width)) {
		BG_Tile (*newTiles)[LEVEL_HEIGHT_TILES] = realloc(tiles,
				sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!newTiles) goto failed;
		tiles = newTiles;

		for (int x = width / BG_TILE_SIZE; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				tiles[x][y] = TILE_CLEAR;
			}
		}

		holeX = hole.x;
		holeY = hole.y;
		projX = proj.startX;
		projY = proj.startY;

		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				BG_DrawTile(bg, tiles[x][y], x * BG_TILE_SIZE,
						y * BG_TILE_SIZE, false);
			}
		}
	} else {
		tiles = calloc(LEVEL_MAX_WIDTH_TILES, sizeof(*tiles));
		if (!tiles) goto failed;

		holeX = holeY = 0;
		projX = 40 + (BG_TILE_SIZE / 2);
		projY = 190 + (BG_TILE_SIZE / 2);
	}

	for (int i = 0; i < NUM_TILES; i++) {
		BG_DrawTile(tileSelectorBG, i, i * (BG_TILE_SIZE + 2), 0, false);
	}

	scroll = 0;
	selectedTile = TILE_CLEAR;
	level = params.editor.level;

	return true;

failed:
	if (bg) BG_Free(bg);
	if (tileSelectorBG) BG_Free(tileSelectorBG);
	if (tiles) free(tiles);
	return false;
}

static bool exportLevel() {
	char path[20];
	sprintf(path, "sdmc:/level_%i.bin", level);

	LevelIO_Hole hole = { holeX, holeY, HOLE_WIDTH, HOLE_HEIGHT };
	LevelIO_Proj proj = { projX, projY, projectileBall };

	int tilesMaxX = 0;
	for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
		for (int x = tilesMaxX; x < LEVEL_MAX_WIDTH_TILES; x++) {
			if (tiles[x][y] != TILE_CLEAR) {
				tilesMaxX = x;
			}
		}
	}

	return LevelIO_Write(path, hole, proj, tiles, (tilesMaxX+1) * BG_TILE_SIZE);
}

static void sceneUpdate() {
	if (BG_IsUpdating(bg)) return;

	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	if (kDown & KEY_B) {
		Scene_SetNext(sceneTitle, Title_MakeParams());
		return;
	}

	if (kHeld & KEY_CPAD_LEFT || kHeld & KEY_CSTICK_LEFT)
		scroll -= SCROLL_UNIT;
	if (kHeld & KEY_CPAD_RIGHT || kHeld & KEY_CSTICK_RIGHT)
		scroll += SCROLL_UNIT;
	scroll = clamp(scroll, 0, LEVEL_MAX_WIDTH - 320);

	if (kDown & KEY_DRIGHT || kDown & KEY_X) {
		selectedTile++;
		if (selectedTile >= NUM_TILES) selectedTile = 0;
	}
	if (kDown & KEY_DLEFT || kDown & KEY_Y) {
		selectedTile--;
		if (selectedTile >= NUM_TILES) selectedTile = NUM_TILES - 1;
	}

	if (TouchInput_InProgress()) {
		float courseX = TouchInput_GetSwipe().end.px + scroll;
		float courseY = TouchInput_GetSwipe().end.py;
		int tileX = courseX / BG_TILE_SIZE;
		int tileY = courseY / BG_TILE_SIZE;

		if (kHeld & KEY_DDOWN || kHeld & KEY_ZR) {
			holeX = tileX * BG_TILE_SIZE;
			holeY = tileY * BG_TILE_SIZE;
		} else if (kHeld & KEY_DUP || kHeld & KEY_R) {
			projX = tileX * BG_TILE_SIZE + (BG_TILE_SIZE / 2);
			projY = tileY * BG_TILE_SIZE + (BG_TILE_SIZE / 2);
		} else {
			int tileX2 = (TouchInput_GetSwipe().start.px + scroll)
					/ BG_TILE_SIZE;
			int tileY2 = (TouchInput_GetSwipe().start.py)
					/ BG_TILE_SIZE;
			int startX = tileX > tileX2 ? tileX2 : tileX;
			int endX = tileX > tileX2 ? tileX : tileX2;
			int startY = tileY > tileY2 ? tileY2 : tileY;
			int endY = tileY > tileY2 ? tileY : tileY2;

			for (int x = startX; x <= endX; x++) {
				for (int y = startY; y <= endY; y++) {
					tiles[x][y] = selectedTile;
					BG_DrawTile(bg, selectedTile,
							x * BG_TILE_SIZE,
							y * BG_TILE_SIZE, true);
				}
			}
		}
	}

	if (kDown & KEY_A) {
		exportLevel();
		Scene_SetNext(sceneTitle, Title_MakeParams());
		return;
	}
}

static void drawRectOutline(int x, int y, int width, int height, u32 color, int 		outlineWidth) {
	C2D_DrawRectSolid(x, y, 0, width, outlineWidth, color);
	C2D_DrawRectSolid(x, y, 0, outlineWidth, height, color);
	C2D_DrawRectSolid(x, y + height - outlineWidth, 0, width, outlineWidth,
			color);
	C2D_DrawRectSolid(x + width - outlineWidth, y, 0, outlineWidth, height,
			color);
}

static void sceneDraw() {
	BG_UpdateGraphics(bg);
	BG_UpdateGraphics(tileSelectorBG);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, C2D_Color32(255, 255, 255, 255));
	C2D_SceneBegin(bottom);

	BG_Draw(tileSelectorBG, 2, 2, 0, 1, 1);
	drawRectOutline(selectedTile * (BG_TILE_SIZE + 2) + 1, 1, BG_TILE_SIZE + 2,
			BG_TILE_SIZE + 2, RED_COLOR, 1);

	C2D_ViewTranslate(-scroll, 0);

	BG_Draw(bg, 0, 0, -1, 1, 1);
	drawRectOutline(holeX, holeY, HOLE_WIDTH, HOLE_HEIGHT, RED_COLOR, 2);
	SpriteSheet_Draw(SPRITE_BALL, projX, projY, 0.5, 0, false, false, NULL);

	C2D_ViewReset();


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, C2D_Color32(255, 255, 255, 255));
	C2D_SceneBegin(top);
}

static void sceneExit() {
	if (bg) BG_Free(bg);
	if (tileSelectorBG) BG_Free(tileSelectorBG);
	if (tiles) free(tiles);
}

Scene sceneEditor = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

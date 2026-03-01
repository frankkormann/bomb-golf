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
#include "../rendering/spritesheet.h"
#include "../projectiles/ball.h"
#include "../util/touchinput.h"
#include "../util/macros.h"
#include "../levelio.h"
#include "../colors.h"

#define HOLE_WIDTH (TILE_SIZE * 2)
#define HOLE_HEIGHT (TILE_SIZE * 4)

#define SCROLL_UNIT TILE_SIZE

static Background bg;
static float scroll;

static Background tileSelectorBG;
static Tile (*tiles)[LEVEL_HEIGHT_TILES];
static SpriteSheet_Sprite selectedTileSprite;
static u8 selectedTileOrientation;

static int holeX, holeY;
static int projX, projY;

static unsigned int level;

Scene_Params Editor_MakeParams(unsigned int level) {
	return (Scene_Params) { .editor = {
		.level = level
	} };
}

static bool sceneInit(Scene_Params params) {
	bg = BG_Create(LEVEL_MAX_WIDTH, LEVEL_HEIGHT, COLOR_BLUE);
	if (!bg) goto failed;

	tileSelectorBG = BG_Create((TILE_SIZE + 2) * NUM_TILES - 2,
			(TILE_SIZE + 2) * 8 - 2, COLOR_BLUE);
	if (!tileSelectorBG) goto failed;

	char path[20];
	sprintf(path, "sdmc:/level_%i.bin", params.editor.level);
	LevelIO_Hole hole;
	LevelIO_Proj proj;
	int width;
	if (LevelIO_Read(path, &hole, &proj, &tiles, &width)) {
		Tile (*newTiles)[LEVEL_HEIGHT_TILES] = realloc(tiles,
				sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!newTiles) goto failed;
		tiles = newTiles;

		for (int x = width / TILE_SIZE; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				tiles[x][y] = Tile_Make(SPRITE_SKY, 0);
			}
		}

		holeX = hole.x;
		holeY = hole.y;
		projX = proj.startX;
		projY = proj.startY;

		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				BG_DrawTile(bg, tiles[x][y], x * TILE_SIZE,
						y * TILE_SIZE, false);
			}
		}
	} else {
		tiles = malloc(sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!tiles) goto failed;

		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				tiles[x][y] = Tile_Make(SPRITE_SKY, 0);
			}
		}

		holeX = holeY = 0;
		projX = 40 + (TILE_SIZE / 2);
		projY = 190 + (TILE_SIZE / 2);
	}

	for (int sprite = 0; sprite < NUM_TILES; sprite++) {
		for (int orientation = 0; orientation < 8; orientation++) {
			BG_DrawTile(tileSelectorBG,
					Tile_Make(sprite + 2, orientation),
					sprite * (TILE_SIZE + 2),
					orientation * (TILE_SIZE + 2), false);
		}
	}

	scroll = 0;
	selectedTileSprite = SPRITE_SKY;
	selectedTileOrientation = 0;
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
			if (Tile_GetSprite(tiles[x][y]) != SPRITE_SKY) {
				tilesMaxX = x;
			}
		}
	}

	return LevelIO_Write(path, hole, proj, tiles, (tilesMaxX+1) * TILE_SIZE);
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

	if (kDown & KEY_DRIGHT) {
		selectedTileSprite++;
		if (selectedTileSprite >= NUM_TILES + FIRST_TILE_SPRITE) {
			selectedTileSprite = FIRST_TILE_SPRITE;
		}
	}
	if (kDown & KEY_DLEFT) {
		selectedTileSprite--;
		if (selectedTileSprite < FIRST_TILE_SPRITE) {
			selectedTileSprite = NUM_TILES + FIRST_TILE_SPRITE - 1;
		}
	}
	if (kDown & KEY_DUP) {
		selectedTileOrientation--;
		if (selectedTileOrientation >= 8) selectedTileOrientation = 7;
	}
	if (kDown & KEY_DDOWN) {
		selectedTileOrientation++;
		if (selectedTileOrientation >= 8) selectedTileOrientation = 0;
	}

	if (TouchInput_InProgress()) {
		float courseX = TouchInput_GetSwipe().end.px + scroll;
		float courseY = TouchInput_GetSwipe().end.py;
		int tileX = courseX / TILE_SIZE;
		int tileY = courseY / TILE_SIZE;

		if (kHeld & KEY_CPAD_DOWN) {
			holeX = tileX * TILE_SIZE;
			holeY = tileY * TILE_SIZE;
		} else if (kHeld & KEY_CPAD_UP) {
			projX = tileX * TILE_SIZE + (TILE_SIZE / 2);
			projY = tileY * TILE_SIZE + (TILE_SIZE / 2);
		} else {
			int tileX2 = (TouchInput_GetSwipe().start.px + scroll)
					/ TILE_SIZE;
			int tileY2 = (TouchInput_GetSwipe().start.py)
					/ TILE_SIZE;
			int startX = tileX > tileX2 ? tileX2 : tileX;
			int endX = tileX > tileX2 ? tileX : tileX2;
			int startY = tileY > tileY2 ? tileY2 : tileY;
			int endY = tileY > tileY2 ? tileY : tileY2;

			for (int x = startX; x <= endX; x++) {
				for (int y = startY; y <= endY; y++) {
					Tile tile = Tile_Make(selectedTileSprite,
							selectedTileOrientation);
					tiles[x][y] = tile;
					BG_DrawTile(bg, tile,
							x * TILE_SIZE,
							y * TILE_SIZE, true);
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
	C2D_TargetClear(bottom, COLOR_WHITE);
	C2D_SceneBegin(bottom);

	C2D_ViewTranslate(-scroll, 0);

	BG_Draw(bg, 0, 0, -1, 1, 1);
	drawRectOutline(holeX, holeY, HOLE_WIDTH, HOLE_HEIGHT, COLOR_DRED, 2);
	SpriteSheet_Draw(SPRITE_BALL, projX, projY, 0.5, 0, false, false, NULL);

	C2D_ViewReset();


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_WHITE);
	C2D_SceneBegin(top);

	BG_Draw(tileSelectorBG, 2, 2, 0, 1, 1);
	drawRectOutline((selectedTileSprite - FIRST_TILE_SPRITE) * (TILE_SIZE+2) + 1,
			selectedTileOrientation * (TILE_SIZE+2) + 1,
			TILE_SIZE + 2, TILE_SIZE + 2, COLOR_DRED, 1);
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

#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "editor.h"
#include "title.h"
#include "components/text.h"
#include "components/tileselector.h"
#include "../rendering/rendertarget.h"
#include "../rendering/background.h"
#include "../rendering/colors.h"
#include "../rendering/spritesheet.h"
#include "../projectiles/ball.h"
#include "../util/touchinput.h"
#include "../util/macros.h"
#include "../util/keychars.h"
#include "../levelio.h"

#define HOLE_WIDTH (TILE_SIZE * 2)
#define HOLE_HEIGHT (TILE_SIZE * 4)

#define SCROLL_UNIT TILE_SIZE

static Background bg;
static float scroll;

static Tile (*tiles)[LEVEL_HEIGHT_TILES];

static int holeX, holeY;
static int projX, projY;
static int par;
static unsigned int level;

static enum { INDIV, FILL } brushType;

static Text infoText;
static C2D_Text controlsText1, controlsText2;
static C2D_TextBuf textBuf;

static Dispatcher touchDispatcher;

Scene_Params Editor_MakeParams(unsigned int level) {
	return (Scene_Params) { .editor = {
		.level = level
	} };
}

// Declaration needed to register with the dispatcher
static bool handleTouchInput(void *ignored);

static bool sceneInit(Scene_Params params) {
	bg = BG_Create(LEVEL_MAX_WIDTH, LEVEL_HEIGHT, COLOR_BLUE);
	if (!bg) goto failed;

	char path[20];
	sprintf(path, "sdmc:/level_%i.bin", params.editor.level);
	LevelIO_Hole hole;
	LevelIO_Proj proj;
	int width;
	if (LevelIO_Read(path, &hole, &proj, &tiles, &width, &par)) {
		Tile (*newTiles)[LEVEL_HEIGHT_TILES] = realloc(tiles,
				sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!newTiles) goto failed;
		tiles = newTiles;

		for (int x = width / TILE_SIZE; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				tiles[x][y] = Tile_Make(SPRITE_TILE_SKY, 0);
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
				tiles[x][y] = Tile_Make(SPRITE_TILE_SKY, 0);
			}
		}

		holeX = holeY = 0;
		projX = 40 + (TILE_SIZE / 2);
		projY = 190 + (TILE_SIZE / 2);
	}

	infoText = Text_Create(50, NULL);
	if (!infoText) goto failed;

	textBuf = C2D_TextBufNew(256);
	if (!textBuf) goto failed;
	C2D_TextParse(&controlsText1, textBuf,
			KEYCHAR_A ": Save and exit\n"
			KEYCHAR_B ": Exit without saving\n"
			KEYCHAR_L ", " KEYCHAR_R ": Switch brush"
		);
	C2D_TextParse(&controlsText2, textBuf,
			KEYCHAR_DPAD "←, →: Change par\n"
			KEYCHAR_DPAD "↑ (hold) + touchscreen: Move ball\n"
			KEYCHAR_DPAD "↓ (hold) + touchscreen: Move hole"
		);
	C2D_TextOptimize(&controlsText1);
	C2D_TextOptimize(&controlsText2);

	if (!TileSelector_Init(Tile_Make(SPRITE_TILE_SKY, 0))) goto failed;

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto failed;
	Dispatcher_AddHandler(touchDispatcher, (Dispatcher_Handler) {
			.priority = 0, NULL, handleTouchInput });
	TileSelector_RegisterForTouchEvents(touchDispatcher, 1);

	scroll = 0;
	level = params.editor.level;
	brushType = INDIV;

	return true;

failed:
	if (bg) BG_Free(bg);
	if (tiles) free(tiles);
	if (infoText) Text_Free(infoText);
	if (touchDispatcher) Dispatcher_Free(touchDispatcher);
	if (textBuf) C2D_TextBufDelete(textBuf);
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
			if (Tile_GetSprite(tiles[x][y]) != SPRITE_TILE_SKY) {
				tilesMaxX = x;
			}
		}
	}

	return LevelIO_Write(path, hole, proj, tiles, (tilesMaxX + 1) * TILE_SIZE,
			par);
}

static void changeTile(int tileX, int tileY, Tile newTile) {
	tiles[tileX][tileY] = newTile;
	BG_DrawTile(bg, newTile, tileX * TILE_SIZE, tileY * TILE_SIZE, true);
}

// ignored param is to match the signature of Dispatcher_Handler
static bool handleTouchInput(void *ignored) {
	u32 kHeld = hidKeysHeld();

	if (TouchInput_InProgress()) {
		float courseX = TouchInput_GetSwipe().end.px + scroll;
		float courseY = TouchInput_GetSwipe().end.py;
		int tileX = courseX / TILE_SIZE;
		int tileY = courseY / TILE_SIZE;

		if (kHeld & KEY_DDOWN) {
			holeX = tileX * TILE_SIZE;
			holeY = tileY * TILE_SIZE;
		} else if (kHeld & KEY_DUP) {
			projX = tileX * TILE_SIZE + (TILE_SIZE / 2);
			projY = tileY * TILE_SIZE + (TILE_SIZE / 2);
		} else if (brushType == FILL) {
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
					changeTile(x, y, TileSelector_GetTile());
				}
			}
		} else if (brushType == INDIV) {
			changeTile(tileX, tileY, TileSelector_GetTile());
		}
	}

	return true;
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

	if (kDown & KEY_DLEFT) par--;
	if (kDown & KEY_DRIGHT) par++;

	if (kDown & KEY_L || kDown & KEY_R) {
		brushType = brushType == INDIV ? FILL : INDIV;
	}

	if (kDown & KEY_A) {
		exportLevel();
		Scene_SetNext(sceneTitle, Title_MakeParams());
		return;
	}

	Dispatcher_DispatchEvent(touchDispatcher);

	Text_SetContent(infoText, "Par: %i\nBrush: %s", par,
			brushType == INDIV ? "Pencil" : "Rectangle");
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
	TileSelector_UpdateGraphics();

	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_WHITE);
	C2D_SceneBegin(bottom);

	TileSelector_Draw(1);

	C2D_ViewTranslate(-scroll, 0);

	BG_Draw(bg, 0, 0, -1, 1, 1);
	drawRectOutline(holeX, holeY, HOLE_WIDTH, HOLE_HEIGHT, COLOR_DRED, 2);
	SpriteSheet_Draw(SPRITE_BALL, projX, projY, 0.5, 0, false, false);

	C2D_ViewReset();


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_WHITE);
	C2D_SceneBegin(top);

	BG_DrawFit(bg, 0, 0, 0, 400, 240);

	C2D_DrawText(&controlsText1, 0, 10, 20, 0, 0.5, 0.5);
	C2D_DrawText(&controlsText2, 0, 160, 20, 0, 0.5, 0.5);
	C2D_DrawText(&infoText->text, 0, 10, 180, 0, 0.5, 0.5);
}

static void sceneExit() {
	BG_Free(bg);
	free(tiles);
	Text_Free(infoText);
	Dispatcher_Free(touchDispatcher);
	TileSelector_Exit();
	C2D_TextBufDelete(textBuf);
}

Scene sceneEditor = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

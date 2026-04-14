#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "editor.h"
#include "levelselector.h"
#include "error.h"
#include "components/text.h"
#include "components/tileselector.h"
#include "components/background.h"
#include "components/border.h"
#include "../rendering/rendertarget.h"
#include "../rendering/colors.h"
#include "../rendering/spritesheet.h"
#include "../projectiles/ball.h"
#include "../util/touchinput.h"
#include "../util/macros.h"
#include "../levelio.h"

#define HOLE_WIDTH (TILE_SIZE * 2)
#define HOLE_HEIGHT (TILE_SIZE * 4)

#define SCROLL_UNIT TILE_SIZE

#define LEVEL_NAME_X 10
#define LEVEL_NAME_Y 15
#define LEVEL_PREVIEW_X 10
#define LEVEL_PREVIEW_Y (LEVEL_NAME_Y + 35) 
#define LEVEL_PREVIEW_WIDTH 380
#define LEVEL_PREVIEW_HEIGHT 90

static Background bg;
static float scroll;

static Tile (*tiles)[LEVEL_HEIGHT_TILES];

static int holeX, holeY;
static int projX, projY;
static int par;
static unsigned int level;

static enum { INDIV, FILL } brushType;

static Text infoText, controlsText1, controlsText2, levelTitleText;

static Dispatcher touchDispatcher;

Scene_Params Editor_MakeParams(unsigned int level) {
	return (Scene_Params) { .editor = {
		.level = level
	} };
}

// Declaration needed to register with the dispatcher
static bool handleTouchInput(void *ignored);

static bool sceneInit(Scene_Params params) {
	char *errMsg = "";

	bg = BG_Create(LEVEL_MAX_WIDTH, LEVEL_HEIGHT, COLOR_BLUE);
	if (!bg) {
		errMsg = "Out of memory";
		goto f_bg;
	}

	levelTitleText = Text_Create(EDITOR_LEVEL_NAME_MAX + 1);
	if (!levelTitleText) {
		errMsg = "Out of memory";
		goto f_levelTitleText;
	}

	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(params.editor.level, false, path);
	LevelIO_Hole hole;
	LevelIO_Proj proj;
	int width;
	char *name;
	if (LevelIO_Read(path, &hole, &proj, &tiles, &width, &par, &name)) {
		Tile (*newTiles)[LEVEL_HEIGHT_TILES] = realloc(tiles,
				sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!newTiles) {
			errMsg = "Out of memory";
			goto f_newTiles;
		}
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

		Text_SetContent(levelTitleText, name);
		free(name);
	} else {
		tiles = malloc(sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!tiles) {
			errMsg = "Out of memory";
			goto f_tiles;
		}

		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				tiles[x][y] = Tile_Make(SPRITE_TILE_SKY, 0);
			}
		}

		holeX = holeY = 0;
		projX = 40 + (TILE_SIZE / 2);
		projY = 190 + (TILE_SIZE / 2);

		Text_SetContent(levelTitleText, "");
	}

	infoText = Text_Create(50);
	if (!infoText) {
		errMsg = "Out of memory";
		goto f_infoText;
	}

	controlsText1 = Text_Create(128);
	if (!controlsText1) {
		errMsg = "Out of memory";
		goto f_controlsText1;
	}
	controlsText2 = Text_Create(128);
	if (!controlsText2) {
		errMsg = "Out of memory";
		goto f_controlsText2;
	}

	Text_SetContent(controlsText1,
			"%c: Save and exit\n"
			"%c: Exit without saving\n"
			"%c/%c: Switch brush",
			TEXT_KEY_A, TEXT_KEY_B, TEXT_KEY_L, TEXT_KEY_R
		);
	Text_SetContent(controlsText2,
			"%c: Change par\n"
			"%c (hold) + touchscreen: Move ball\n"
			"%c (hold) + touchscreen: Move hole",
			TEXT_KEY_DPAD, TEXT_KEY_DUP, TEXT_KEY_DDOWN
		);

	if (!TileSelector_Init(Tile_Make(SPRITE_TILE_SKY, 0))) {
		errMsg = "Out of memory";
		goto f_TileSelector;
	}

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) {
		errMsg = "Out of memory";
		goto f_touchDispatcher;
	}
	Dispatcher_AddHandler(touchDispatcher, (Dispatcher_Handler) {
			.priority = 0, NULL, handleTouchInput });
	TileSelector_RegisterForTouchEvents(touchDispatcher, 1);

	scroll = 0;
	level = params.editor.level;
	brushType = INDIV;

	return true;

f_touchDispatcher:
	TileSelector_Exit();
f_TileSelector:
	Text_Free(controlsText2);
f_controlsText2:
	Text_Free(controlsText1);
f_controlsText1:
	Text_Free(infoText);
f_infoText:
f_newTiles:
	free(tiles);
f_tiles:
	Text_Free(levelTitleText);
f_levelTitleText:
	BG_Free(bg);
f_bg:
	Scene_SetNext(sceneError, Error_MakeParams(errMsg));
	return false;
}

static void sceneExit() {
	BG_Free(bg);
	free(tiles);
	Text_Free(infoText);
	Text_Free(controlsText1);
	Text_Free(controlsText2);
	Text_Free(levelTitleText);
	Dispatcher_Free(touchDispatcher);
	TileSelector_Exit();
}

static bool exportLevel() {
	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(level, false, path);

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

	SwkbdState keyboard;
	SwkbdButton pressedButton;
	char buf[EDITOR_LEVEL_NAME_MAX + 1];
	swkbdInit(&keyboard, SWKBD_TYPE_QWERTY, 2, EDITOR_LEVEL_NAME_MAX);
	swkbdSetValidation(&keyboard, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetHintText(&keyboard, "Enter a name");

	pressedButton = swkbdInputText(&keyboard, buf, EDITOR_LEVEL_NAME_MAX + 1);
	if (pressedButton != SWKBD_BUTTON_CONFIRM) {
		return false;
	}

	return LevelIO_Write(path, hole, proj, tiles, (tilesMaxX + 1) * TILE_SIZE,
			par, buf);
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
		Scene_SetNext(sceneLevelSelector, LevelSelector_MakeParams());
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
		if (exportLevel()) {
			Scene_SetNext(sceneLevelSelector,
					LevelSelector_MakeParams());
			return;
		} else {
			//TODO Figure out better solution than kicking user out
			Scene_SetNext(sceneError,
					Error_MakeParams("Failed to save file"));
			return;
		}
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
	SpriteSheet_DrawCentered(SPRITE_BALL, projX, projY, 0.5, 0, false, false);

	C2D_ViewReset();


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	BG_DrawFit(bg, LEVEL_PREVIEW_X, LEVEL_PREVIEW_Y, 0, LEVEL_PREVIEW_WIDTH,
			LEVEL_PREVIEW_HEIGHT);
	Border_Draw(LEVEL_PREVIEW_X, LEVEL_PREVIEW_Y, 0, LEVEL_PREVIEW_WIDTH,
			LEVEL_PREVIEW_HEIGHT);

	Text_Draw(controlsText1, 10, 14, 0, COLOR_DGREEN, 1);
	Text_Draw(controlsText2, 160, 14, 0, COLOR_DGREEN, 1);
	Text_Draw(infoText, 10, 180, 0, COLOR_DGREEN, 1);
	Text_Draw(levelTitleText, 10, 220, 0, COLOR_DGREEN, 1);
}

Scene sceneEditor = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

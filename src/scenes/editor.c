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
#include "components/button.h"
#include "components/editormenu.h"
#include "components/brushselector.h"
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

#define TEXT_MARGIN 10
#define LEVEL_NAME_Y 15
#define LEVEL_PREVIEW_X 10
#define LEVEL_PREVIEW_Y (LEVEL_NAME_Y + 35) 
#define LEVEL_PREVIEW_WIDTH 380
#define LEVEL_PREVIEW_HEIGHT 90
#define CONTROLS_TEXT_Y (LEVEL_PREVIEW_Y + LEVEL_PREVIEW_HEIGHT + 15)

static Background bg;
static float scroll;

static Tile (*tiles)[LEVEL_HEIGHT_TILES];

static int holeX, holeY;
static int projX, projY;
static int par;
static int level;
static char *name;

static Text nameText, parText;

static Dispatcher touchDispatcher;

Scene_Params Editor_MakeParams(unsigned int level) {
	return (Scene_Params) { .editor = {
		.level = level
	} };
}

// Declarations needed to register with dispatcher, buttons
static bool handleTouchInput(void *ignored);
static void editName(void *ignored);
static void saveExit(void *ignored);
static void exitNoSave(void *ignored);
static void changePar(int change);

static bool sceneInit(Scene_Params params) {
	char *errMsg = "";  // Fill this in whenever you goto f_???

	bg = BG_Create(LEVEL_MAX_WIDTH, LEVEL_HEIGHT, COLOR_BLUE);
	if (!bg) {
		errMsg = "Out of memory";
		goto f_bg;
	}

	nameText = Text_Create(EDITOR_LEVEL_NAME_MAX + 1);
	if (!nameText) {
		errMsg = "Out of memory";
		goto f_nameText;
	}

	parText = Text_Create(9);
	if (!parText) {
		errMsg = "Out of memory";
		goto f_parText;
	}

	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(params.editor.level, false, path);
	LevelIO_Hole hole;
	LevelIO_Proj proj;
	int width;
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
		name = malloc(sizeof('\0'));
		name[0] = '\0';
	}
	Text_SetContent(nameText, name);

	if (!TileSelector_Init(Tile_Make(SPRITE_TILE_GRASS, 0))) {
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
	TileSelector_RegisterForTouchEvents(touchDispatcher, 2);

	if (!EditorMenu_Init(editName, saveExit, exitNoSave, changePar)) {
		errMsg = "Out of memory";
		goto f_EditorMenu;
	}
	EditorMenu_RegisterForTouchEvents(touchDispatcher, 3);

	if (!BrushSelector_Init(BRUSH_PENCIL)) {
		errMsg = "Out of memory";
		goto f_BrushSelector;
	}
	BrushSelector_RegisterForTouchEvents(touchDispatcher, 1);

	scroll = 0;
	level = params.editor.level;

	return true;

f_BrushSelector:
	EditorMenu_Exit();
f_EditorMenu:
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	TileSelector_Exit();
f_TileSelector:
f_newTiles:
	free(tiles);
f_tiles:
	Text_Free(parText);
f_parText:
	Text_Free(nameText);
f_nameText:
	BG_Free(bg);
f_bg:
	Scene_SetNext(sceneError, Error_MakeParams(errMsg));
	return false;
}

static void sceneExit() {
	BG_Free(bg);
	free(tiles);
	free(name);
	Text_Free(nameText);
	Text_Free(parText);
	Dispatcher_Free(touchDispatcher);
	TileSelector_Exit();
	EditorMenu_Exit();
	BrushSelector_Exit();
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

	return LevelIO_Write(path, hole, proj, tiles, (tilesMaxX + 1) * TILE_SIZE,
			par, name);
}

static void changeTile(int tileX, int tileY, Tile newTile) {
	tiles[tileX][tileY] = newTile;
	BG_DrawTile(bg, newTile, tileX * TILE_SIZE, tileY * TILE_SIZE, true);
}

// ignored param is to match the signature of Dispatcher_Handler
static bool handleTouchInput(void *ignored) {
	if (!TouchInput_InProgress()) return false;

	float courseX = TouchInput_GetSwipe().end.px + scroll;
	float courseY = TouchInput_GetSwipe().end.py;
	int tileX = courseX / TILE_SIZE;
	int tileY = courseY / TILE_SIZE;

	switch (BrushSelector_GetBrush()) {
		case BRUSH_PENCIL:
			changeTile(tileX, tileY, TileSelector_GetTile());
			break;
		case BRUSH_RECTANGLE:
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
			break;
		case BRUSH_BALL_POS:
			projX = tileX * TILE_SIZE + (TILE_SIZE / 2);
			projY = tileY * TILE_SIZE + (TILE_SIZE / 2);
			break;
		case BRUSH_HOLE_POS:
			holeX = tileX * TILE_SIZE;
			holeY = tileY * TILE_SIZE;
			break;
	}

	return true;
}

// ignored param is to match the signature of Button callback
static void editName(void *ignored) {
	SwkbdState keyboard;
	SwkbdButton pressedButton;
	char buf[EDITOR_LEVEL_NAME_MAX + 1];
	swkbdInit(&keyboard, SWKBD_TYPE_QWERTY, 2, EDITOR_LEVEL_NAME_MAX);
	swkbdSetValidation(&keyboard, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetHintText(&keyboard, "Enter a name");
	swkbdSetInitialText(&keyboard, name);

	pressedButton = swkbdInputText(&keyboard, buf, EDITOR_LEVEL_NAME_MAX + 1);
	if (pressedButton != SWKBD_BUTTON_CONFIRM) {
		return;
	}

	char *newName = realloc(name, sizeof(char) * (strlen(buf) + 1));
	if (!newName) return;
	name = newName;
	strcpy(name, buf);
	Text_SetContent(nameText, name);
}

// ignored param is to match the signature of Button callback
static void saveExit(void *ignored) {
	if (exportLevel()) {
		Scene_SetNext(sceneLevelSelector, LevelSelector_MakeParams(level));
		return;
	} else {
		//TODO Figure out better solution than kicking user out
		Scene_SetNext(sceneError, Error_MakeParams("Failed to save file"));
		return;
	}
}

// ignored param is to match the signature of Button callback
static void exitNoSave(void *ignored) {
	Scene_SetNext(sceneLevelSelector, LevelSelector_MakeParams(level));
}

static void changePar(int change) {
	par += change;
}

static void sceneUpdate() {
	if (BG_IsUpdating(bg)) return;

//	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	if (kHeld & KEY_CPAD_LEFT || kHeld & KEY_CSTICK_LEFT)
		scroll -= SCROLL_UNIT;
	if (kHeld & KEY_CPAD_RIGHT || kHeld & KEY_CSTICK_RIGHT)
		scroll += SCROLL_UNIT;
	scroll = clamp(scroll, 0, LEVEL_MAX_WIDTH - 320);

	Dispatcher_DispatchEvent(touchDispatcher);

	Text_SetContent(parText, "Par %i", par);
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


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	Text_Draw(nameText, TEXT_MARGIN, LEVEL_NAME_Y, 0, COLOR_DGREEN, 1);
	Text_DrawRight(parText, 390, LEVEL_NAME_Y + TEXT_LINE_HEIGHT, 0,
			COLOR_DGREEN, 1);
	BG_Rectangle bgPos = BG_DrawFit(bg, LEVEL_PREVIEW_X, LEVEL_PREVIEW_Y, 0,
			LEVEL_PREVIEW_WIDTH, LEVEL_PREVIEW_HEIGHT);
	Border_Draw(bgPos.x, bgPos.y, 0, bgPos.width, bgPos.height);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_WHITE);
	C2D_SceneBegin(bottom);

	C2D_ViewTranslate(-scroll, 0);

	BG_Draw(bg, 0, 0, -1, 1, 1);
	drawRectOutline(holeX, holeY, HOLE_WIDTH, HOLE_HEIGHT, COLOR_DRED, 2);
	SpriteSheet_DrawCentered(SPRITE_BALL, projX, projY, 0.5, 0, false, false);

	C2D_ViewReset();

	TileSelector_Draw(0.5);
	BrushSelector_Draw(0.4);
	EditorMenu_Draw(1);
}

Scene sceneEditor = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

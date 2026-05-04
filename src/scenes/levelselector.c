#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "levelselector.h"
#include "title.h"
#include "components/levelcard.h"
#include "components/text.h"
#include "components/background.h"
#include "components/border.h"
#include "../rendering/rendertarget.h"
#include "../rendering/colors.h"
#include "../rendering/spritesheet.h"
#include "../util/dispatcher.h"
#include "../levelio.h"

#define LEVEL_NAME_X 10
#define LEVEL_NAME_Y 15

#define LEVEL_PREVIEW_X 10
#define LEVEL_PREVIEW_Y (LEVEL_NAME_Y + 35)
#define LEVEL_PREVIEW_WIDTH 380
#define LEVEL_PREVIEW_HEIGHT 90

#define NUM_LEVEL_ROWS 6
#define NUM_LEVEL_COLUMNS 3

#define CARD_X_START 12
#define CARD_Y_START 12
#define CARD_GAP_X 50
#define CARD_GAP_Y 73

static Dispatcher touchDispatcher;
static LevelCard levelCards[NUM_LEVEL_ROWS][NUM_LEVEL_COLUMNS];

static Text nameText, parText, infoText;
static Background levelPreview;
static bool levelIsSelected;

Scene_Params LevelSelector_MakeParams(int level) {
	return (Scene_Params) { .levelselector = {
		.level = level
	} };
}

static void displayLevel(int levelNum) {
	if (levelNum < 0) {
		Text_SetContent(infoText, "Tap a level number to preview");
		levelIsSelected = false;
	} else {
		char path[LEVEL_PATH_MAX];
		LevelIO_MakePath(levelNum, false, path);
		LevelIO_Hole hole;
		LevelIO_Proj proj;
		Tile (*tiles)[LEVEL_HEIGHT_TILES];
		int width, par;
		char *name;

		if (!LevelIO_Read(path, &hole, &proj, &tiles, &width, &par, &name)) {
			// Spaces to maintain center alignment
			Text_SetContent(infoText, "     Level does not exist");
			levelIsSelected = false;
			return;
		}
		BG_ClearAll(levelPreview);
		for (int x = 0; x < width / TILE_SIZE; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				BG_DrawTile(levelPreview, tiles[x][y], x * TILE_SIZE,
						y * TILE_SIZE, false);
			}
		}
		free(tiles);

		Text_SetContent(nameText, "%s", name);
		free(name);
		Text_SetContent(parText, "Par %i", par);
		levelIsSelected = true;
	}
}

static bool sceneInit(Scene_Params params) {
	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	for (size_t r = 0; r < NUM_LEVEL_ROWS; r++) {
		for (size_t c = 0; c < NUM_LEVEL_COLUMNS; c++) {
			levelCards[r][c] = LevelCard_Create(
					CARD_X_START + (CARD_GAP_X) * r,
					CARD_Y_START + (CARD_GAP_Y) * c,
					r + c * NUM_LEVEL_ROWS,
					displayLevel
				);
			if (!levelCards[r][c]) goto f_levelCards;
			if (!LevelCard_RegisterForTouchEvents(levelCards[r][c],
					touchDispatcher, 0)) {
				goto f_levelCards;
			}
		}
	}

	nameText = Text_Create(EDITOR_LEVEL_NAME_MAX + 1);
	if (!nameText) goto f_nameText;

	parText = Text_Create(9);
	if (!parText) goto f_parText;

	infoText = Text_Create(32);
	if (!infoText) goto f_infoText;

	levelPreview = BG_Create(LEVEL_MAX_WIDTH, LEVEL_HEIGHT, COLOR_BLUE);
	if (!levelPreview) goto f_levelPreview;

	displayLevel(params.levelselector.level);

	return true;

f_levelPreview:
	Text_Free(infoText);
f_infoText:
	Text_Free(parText);
f_parText:
	Text_Free(nameText);
f_nameText:
f_levelCards:
	for (size_t r = 0; r < NUM_LEVEL_ROWS; r++) {
		for (size_t c = 0; c < NUM_LEVEL_COLUMNS; c++) {
			if (levelCards[r][c]) LevelCard_Free(levelCards[r][c]);
		}
	}
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	Scene_SetNext(sceneError, Error_MakeParams("Out of memory"));
	return false;
}

static void sceneExit() {
	Dispatcher_Free(touchDispatcher);
	for (size_t r = 0; r < NUM_LEVEL_ROWS; r++) {
		for (size_t c = 0; c < NUM_LEVEL_COLUMNS; c++) {
			LevelCard_Free(levelCards[r][c]);
		}
	}
	Text_Free(nameText);
	Text_Free(parText);
	Text_Free(infoText);
	BG_Free(levelPreview);
}

static void sceneUpdate() {
	u32 kDown = hidKeysDown();

	if (kDown & KEY_B) {
		Scene_SetNext(sceneTitle, Title_MakeParams());
		return;
	}

	Dispatcher_DispatchEvent(touchDispatcher);
}

static void sceneDraw() {
	BG_UpdateGraphics(levelPreview);

	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	if (levelIsSelected) {
		Text_Draw(nameText, LEVEL_NAME_X, LEVEL_NAME_Y, 0, COLOR_DGREEN, 1,
				TEXT_LEFT);
		Text_Draw(parText, 390, LEVEL_NAME_Y, 0, COLOR_DGREEN, 1,
				TEXT_RIGHT);
		BG_Rectangle bgPos = BG_DrawFit(levelPreview, LEVEL_PREVIEW_X,
				LEVEL_PREVIEW_Y, 0, LEVEL_PREVIEW_WIDTH,
				LEVEL_PREVIEW_HEIGHT);
		Border_Draw(bgPos.x, bgPos.y, 0, bgPos.width, bgPos.height);
	} else {
		Text_Draw(infoText, 105, 60, 0, COLOR_DGRAY, 1, TEXT_LEFT);
	}


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	for (size_t r = 0; r < NUM_LEVEL_ROWS; r++) {
		for (size_t c = 0; c < NUM_LEVEL_COLUMNS; c++) {
			LevelCard_Draw(levelCards[r][c], 0);
		}
	}
}

Scene sceneLevelSelector = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

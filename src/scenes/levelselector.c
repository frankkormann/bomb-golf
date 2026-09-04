#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "levelselector.h"
#include "title.h"
#include "components/levelcard.h"
#include "components/background.h"
#include "components/text.h"
#include "components/border.h"
#include "../environment/terrain.h"
#include "../rendering/rendertarget.h"
#include "../rendering/colors.h"
#include "../rendering/spritesheet.h"
#include "../rendering/animation.h"
#include "../rendering/draw3d.h"
#include "../util/dispatcher.h"
#include "../levelio.h"

#define LEVEL_NAME_X		10
#define LEVEL_NAME_Y		15

#define LEVEL_PREVIEW_X		10
#define LEVEL_PREVIEW_Y		(LEVEL_NAME_Y + 35)
#define LEVEL_PREVIEW_WIDTH	380
#define LEVEL_PREVIEW_HEIGHT	90

#define NUM_LEVEL_ROWS		6
#define NUM_LEVEL_COLUMNS	3

#define CARD_X_START		12
#define CARD_Y_START		12
#define CARD_GAP_X		50
#define CARD_GAP_Y		73

static Dispatcher touchDispatcher;
static LevelCard levelCards[NUM_LEVEL_ROWS][NUM_LEVEL_COLUMNS];

static Text nameText, parText, infoText;
static Background levelPreview;
static LevelIO_Obst *obstacles;
static size_t numObstacles;
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
		if (obstacles) {
			// In case we had it from a previous level selection
			free(obstacles);
			obstacles = NULL;
		}

		char path[LEVEL_PATH_MAX];
		LevelIO_MakePath(levelNum, false, path);
		Tile (*tiles)[LEVEL_HEIGHT_TILES];
		Tile_WithPos *overlayTiles;
		size_t numOverlayTiles;
		int width, par;
		char *name;

		if (!LevelIO_Read(path, NULL, NULL, &tiles, &overlayTiles,
				&numOverlayTiles, &obstacles, &numObstacles, &width,
				&par, &name, NULL)) {
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
		for (size_t i = 0; i < numOverlayTiles; i++) {
			int x, y;
			Tile_GetPos(overlayTiles[i], &x, &y);
			BG_DrawTile(levelPreview, overlayTiles[i], x, y, false);
		}

		free(tiles);
		free(overlayTiles);

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
	if (obstacles) {
		free(obstacles);
		obstacles = NULL;
	}
}

static void sceneUpdate(float _) {
	u32 kDown = hidKeysDown();

	if (kDown & KEY_B) {
		Scene_SetNext(sceneTitle, Title_MakeParams());
		return;
	}

	Dispatcher_DispatchEvent(touchDispatcher);
}

static void drawObstacle(LevelIO_Obst obst, float previewX, float previewY,
		float previewWidth, float previewHeight, float depth) {
	C3D_Mtx prevMtx;
	C2D_ViewSave(&prevMtx);
	C2D_ViewTranslate(previewX, previewY);
	C2D_ViewScale(previewWidth / LEVEL_MAX_WIDTH, previewHeight / LEVEL_HEIGHT);

	int firstX = obst.xs[0];
	int firstY = obst.ys[0];
	int secondX = obst.numPoints > 1 ? obst.xs[1] : obst.xs[0];
	int secondY = obst.numPoints > 1 ? obst.ys[1] : obst.ys[0];

	SpriteSheet_DrawObstacle(
			obst.sprite1,
			firstX,
			firstY,
			0.5,
			firstX == secondX && firstY != secondY ? M_PI/2 : 0,
			firstX > secondX || (firstX == secondX && firstY > secondY),
			firstX == secondX && firstY != secondY
		);

	C2D_ViewRestore(&prevMtx);
}

static void sceneDraw() {
	BG_UpdateGraphics(levelPreview);

	int previewX, previewY, previewWidth, previewHeight;
	#define D3D_DEPTHS { 0.5, 0.5, 0, 0.5, 0.5, 0.5 }
	#define D3D_XS { \
			LEVEL_NAME_X, \
			390, \
			LEVEL_PREVIEW_X, \
			previewX + D3D_CORRECTION(3), \
			previewX, \
			105, \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, COLOR_LGRAY); \
	C2D_SceneBegin(D3D_TARGET); \
	\
	if (levelIsSelected) { \
		Text_Draw(nameText, D3D_X(0), LEVEL_NAME_Y, D3D_D(0), COLOR_DGREEN, \
				1, TEXT_LEFT); \
		Text_Draw(parText, D3D_X(1), LEVEL_NAME_Y, D3D_D(1), COLOR_DGREEN, \
				1, TEXT_RIGHT); \
		\
		BG_DrawFit(levelPreview, D3D_X(2), LEVEL_PREVIEW_Y, D3D_D(2), \
				LEVEL_PREVIEW_WIDTH, LEVEL_PREVIEW_HEIGHT, \
				&previewX, &previewY, &previewWidth, \
				&previewHeight); \
		Border_Draw(D3D_X(3), previewY, D3D_D(3), \
				previewWidth - 2*D3D_CORRECTION(3), \
				previewHeight); \
		\
		for (size_t i = 0; i < numObstacles; i++) { \
			drawObstacle(obstacles[i], D3D_X(4), previewY, \
					previewWidth, previewHeight, D3D_D(4)); \
		} \
	} else { \
		Text_Draw(infoText, D3D_X(5), 60, D3D_D(5), COLOR_DGRAY, 1, \
			TEXT_LEFT); \
	}
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	for (size_t r = 0; r < NUM_LEVEL_ROWS; r++) {
		for (size_t c = 0; c < NUM_LEVEL_COLUMNS; c++) {
			LevelCard_Draw(levelCards[r][c], 0);
		}
	}
	Animation_Draw(0.5);
}

Scene sceneLevelSelector = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

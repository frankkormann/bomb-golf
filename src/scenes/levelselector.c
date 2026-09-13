#include <stdbool.h>
#include <malloc.h>
#include <stdio.h>
#include <math.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "levelselector.h"
#include "title.h"
#include "error.h"
#include "editor.h"
#include "course.h"
#include "components/background.h"
#include "components/text.h"
#include "components/border.h"
#include "components/popup.h"
#include "components/button.h"
#include "../environment/terrain.h"
#include "../rendering/rendertarget.h"
#include "../rendering/color.h"
#include "../rendering/spritesheet.h"
#include "../rendering/animation.h"
#include "../rendering/draw3d.h"
#include "../util/dispatcher.h"
#include "../util/tracker.h"
#include "../savedata.h"
#include "../levelio.h"

#define LEVEL_NAME_X		10
#define LEVEL_NAME_Y		15
#define LEVEL_PREVIEW_X		10
#define LEVEL_PREVIEW_Y		(LEVEL_NAME_Y + 35)
#define LEVEL_PREVIEW_WIDTH	380
#define LEVEL_PREVIEW_HEIGHT	90

#define BUTTON_Y		-2
#define PLAY_BUTTON_X		10
#define PLAYSEQ_BUTTON_X	60
#define EDIT_BUTTON_X		162
#define COPY_BUTTON_X		212
#define DELETE_BUTTON_X		262
#define NUM_CARD_ROWS		3
#define NUM_CARD_COLS		6
#define CARD_X_START		31
#define CARD_Y_START		45
#define CARD_WIDTH		38
#define CARD_HEIGHT		45
#define CARD_X_GAP		(CARD_WIDTH + 6)
#define CARD_Y_GAP		(CARD_HEIGHT + 5)

static Dispatcher touchDispatcher;
static Button playButton, playSeqButton, editButton, copySwapButton, deleteButton;
static Text   playText,   playSeqText,   editText,   copySwapText,   deleteText;

static Button cards[NUM_CARD_ROWS * NUM_CARD_COLS];
static Text cardNumbers[NUM_CARD_ROWS * NUM_CARD_COLS];

static Text nameText, parText, infoText;
static Background levelPreview;
static LevelIO_Obst *obstacles;
static size_t numObstacles;
static int selectedLevel;
static bool isLevelLoaded, inCopyMode;

// Declarations needed for buttons
static void play();
static void playSequence();
static void edit();
static void copySwap();
static void delete();
static void select(int level);

static bool sceneInit(void *sceneParams) {
	LevelSelector_Params *params = (LevelSelector_Params*)sceneParams;

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	playButton = Button_Create(PLAY_BUTTON_X, BUTTON_Y, SPRITE_SMALL_BUTTON,
			-1, NULL, play);
	if (!playButton) goto f_playButton;

	playSeqButton = Button_Create(PLAYSEQ_BUTTON_X, BUTTON_Y,
			SPRITE_MEDIUM_BUTTON, -1, NULL, playSequence);
	if (!playSeqButton) goto f_playSeqButton;

	editButton = Button_Create(EDIT_BUTTON_X, BUTTON_Y, SPRITE_SMALL_BUTTON,
			-1, NULL, edit);
	if (!editButton) goto f_editButton;

	copySwapButton = Button_Create(COPY_BUTTON_X, BUTTON_Y, SPRITE_SMALL_BUTTON,
			-1, NULL, copySwap);
	if (!copySwapButton) goto f_copySwapButton;

	deleteButton = Button_Create(DELETE_BUTTON_X, BUTTON_Y, SPRITE_SMALL_BUTTON,
			-1, NULL, delete);
	if (!deleteButton) goto f_deleteButton;

	Button_RegisterForTouchEvents(playButton, touchDispatcher, 0);
	Button_RegisterForTouchEvents(playSeqButton, touchDispatcher, 0);
	Button_RegisterForTouchEvents(editButton, touchDispatcher, 0);
	Button_RegisterForTouchEvents(copySwapButton, touchDispatcher, 0);
	Button_RegisterForTouchEvents(deleteButton, touchDispatcher, 0);

	playText = Text_Create(5);
	if (!playText) goto f_playText;
	Text_SetContent(playText, "Play");

	playSeqText = Text_Create(16);
	if (!playSeqText) goto f_playSeqText;
	Text_SetContent(playSeqText, "Play In Order");

	editText = Text_Create(5);
	if (!editText) goto f_editText;
	Text_SetContent(editText, "Edit");

	copySwapText = Text_Create(8);
	if (!copySwapText) goto f_copySwapText;

	deleteText = Text_Create(7);
	if (!deleteText) goto f_deleteText;
	Text_SetContent(deleteText, "Erase");
	
	int i = 0;
	for (int k = 0; k < NUM_CARD_ROWS; k++) {
		for (int j = 0; j < NUM_CARD_COLS; j++, i++) {
			float cardX = CARD_X_START + j*CARD_X_GAP;
			float cardY = CARD_Y_START + k*CARD_Y_GAP;
			cards[i] = Button_Create(cardX, cardY, SPRITE_LEVEL_CARD,
					-1, (void*)i, (void(*)(void*))select);
			if (!cards[i]) goto f_cards;
			Button_RegisterForTouchEvents(cards[i], touchDispatcher, 0);
		}
	}

	int j;
	for (j = 0; j < NUM_CARD_ROWS * NUM_CARD_COLS; j++) {
		cardNumbers[j] = Text_Create(3);
		if (!cardNumbers[j]) goto f_cardNumbers;
		Text_SetContent(cardNumbers[j], "%i", j + 1);
	}

	nameText = Text_Create(EDITOR_LEVEL_NAME_MAX + 1);
	if (!nameText) goto f_nameText;

	parText = Text_Create(9);
	if (!parText) goto f_parText;

	infoText = Text_Create(32);
	if (!infoText) goto f_infoText;

	levelPreview = BG_Create(LEVEL_MAX_WIDTH, LEVEL_HEIGHT, COLOR_BLUE);
	if (!levelPreview) goto f_levelPreview;

	inCopyMode = false;
	select(params->level);

	return true;

f_levelPreview:
	Text_Free(infoText);
f_infoText:
	Text_Free(parText);
f_parText:
	Text_Free(nameText);
f_nameText:
f_cardNumbers:
	for (int k = 0; k < j; k++) Text_Free(cardNumbers[k]);
f_cards:
	for (int k = 0; k < i; k++) Button_Free(cards[k]);
	Text_Free(deleteText);
f_deleteText:
	Text_Free(copySwapText);
f_copySwapText:
	Text_Free(editText);
f_editText:
	Text_Free(playSeqText);
f_playSeqText:
	Text_Free(playText);
f_playText:
	Button_Free(deleteButton);
f_deleteButton:
	Button_Free(copySwapButton);
f_copySwapButton:
	Button_Free(editButton);
f_editButton:
	Button_Free(playSeqButton);
f_playSeqButton:
	Button_Free(playButton);
f_playButton:
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	Scene_Switch(sceneError, &(Error_Params) { "Out of memory" });
	return false;
}

static void sceneExit() {
	Text_Free(infoText);
	Text_Free(parText);
	Text_Free(nameText);
	Text_Free(deleteText);
	Text_Free(copySwapText);
	Text_Free(editText);
	Text_Free(playSeqText);
	Text_Free(playText);
	Button_Free(deleteButton);
	Button_Free(copySwapButton);
	Button_Free(editButton);
	Button_Free(playSeqButton);
	Button_Free(playButton);
	Dispatcher_Free(touchDispatcher);
	BG_Free(levelPreview);
	if (obstacles) {
		free(obstacles);
		obstacles = NULL;
	}
	for (int i = 0; i < NUM_CARD_ROWS * NUM_CARD_COLS; i++) {
		Text_Free(cardNumbers[i]);
	}
}

static void play() {
	if (isLevelLoaded) {
		Tracker_Clear();
		Scene_Switch(sceneCourse, &(Course_Params) { selectedLevel, false });
	}
}

static void playSequence() {
	//FIXME
}

static void edit() {
	Scene_Switch(sceneEditor, &(Editor_Params) { selectedLevel });
}

static void copySwap() {
	if (selectedLevel < 0) return;
	if (!inCopyMode) {
		inCopyMode = true;
		Text_SetContent(infoText, "Tap another level number");
		Text_SetContent(copySwapText, "Cancel");
	} else {
		inCopyMode = false;
		select(selectedLevel);
	}
}

static void doDelete() {
	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(selectedLevel, false, path);
	remove(path);
	select(selectedLevel);
	Popup_Exit();
}

static void delete() {
	Popup_Button buttons[] = {
			{ "Erase", -1, NULL, doDelete },
			{ "Cancel", KEY_B, NULL, Popup_Exit }
		};
	Popup_Init("Really delete? This can't be undone.", POPUP_TWO_BUTTON,
			buttons);
}

static void display(int level) {
	selectedLevel = level;
	if (level < 0) {
		Text_SetContent(infoText, "Tap a level number to preview");
		isLevelLoaded = false;
		Text_SetContent(copySwapText, "Copy");
	} else {
		if (obstacles) {
			// In case we had it from a previous level selection
			free(obstacles);
			obstacles = NULL;
		}

		char path[LEVEL_PATH_MAX];
		LevelIO_MakePath(level, false, path);
		Tile (*tiles)[LEVEL_HEIGHT_TILES];
		Tile_WithPos *overlayTiles;
		size_t numOverlayTiles;
		int width, par;
		char *name;

		if (!LevelIO_Read(path, NULL, NULL, &tiles, &overlayTiles,
				&numOverlayTiles, &obstacles, &numObstacles, &width,
				&par, &name, NULL)) {
			// Spaces to maintain center alignment
			Text_SetContent(infoText, "Level does not exist");
			isLevelLoaded = false;
			Text_SetContent(copySwapText, "Copy");
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
		isLevelLoaded = true;
		Text_SetContent(copySwapText, "Swap");
	}
}

static void select(int level) {
	if (!inCopyMode) {
		display(level);
	} else {
		char oldPath[LEVEL_PATH_MAX], newPath[LEVEL_PATH_MAX];
		LevelIO_MakePath(selectedLevel, false, oldPath);
		LevelIO_MakePath(level, false, newPath);
		if (isLevelLoaded) {
			SaveData_Swap(newPath, oldPath);
		} else {
			SaveData_Copy(oldPath, newPath);
		}
		inCopyMode = false;
		display(selectedLevel);
	}
}

static void sceneUpdate(float _) {
	u32 kDown = hidKeysDown();

	if (kDown & KEY_B) {
		Scene_Switch(sceneTitle, &(Title_Params) SCENE_PARAMS_EMPTY);
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

static void drawOutline(int x, int y, float depth, int width, int height,
		u32 color, int outlineWidth) {
	C2D_DrawRectSolid(x+1, y, depth, width-2, outlineWidth, color);
	C2D_DrawRectSolid(x, y+1, depth, outlineWidth, height-2, color);
	C2D_DrawRectSolid(x+1, y + height - outlineWidth, 0, width-2, outlineWidth,
			color);
	C2D_DrawRectSolid(x + width - outlineWidth, y+1, 0, outlineWidth, height-2,
			color);
}

static void sceneDraw() {
	BG_UpdateGraphics(levelPreview);

	int previewX, previewY, previewWidth, previewHeight;
	#define D3D_DEPTHS { 0.8, 0.8, 0, 0.6, 0.8, 0.8 }
	#define D3D_XS { \
			LEVEL_NAME_X, \
			390, \
			LEVEL_PREVIEW_X, \
			previewX + ceilf(D3D_CORRECTION(3)), \
			previewX, \
			200, \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, COLOR_LGRAY); \
	C2D_SceneBegin(D3D_TARGET); \
	\
	if (isLevelLoaded && !inCopyMode) { \
		Text_Draw(nameText, D3D_Xi(0), LEVEL_NAME_Y, D3D_D(0), \
				COLOR_DGREEN, 1, TEXT_LEFT); \
		Text_Draw(parText, D3D_Xi(1), LEVEL_NAME_Y, D3D_D(1), COLOR_DGREEN, \
				1, TEXT_RIGHT); \
		\
		BG_DrawFit(levelPreview, D3D_Xi(2), LEVEL_PREVIEW_Y, D3D_D(2), \
				LEVEL_PREVIEW_WIDTH, LEVEL_PREVIEW_HEIGHT, \
				&previewX, &previewY, &previewWidth, \
				&previewHeight); \
		Border_DrawLight(D3D_Xi(3), previewY, D3D_D(3), \
				previewWidth - 2*ceilf(D3D_CORRECTION(3)), \
				previewHeight); \
		\
		for (size_t i = 0; i < numObstacles; i++) { \
			drawObstacle(obstacles[i], D3D_X(4), previewY, \
					previewWidth, previewHeight, D3D_D(4)); \
		} \
	} else { \
		Text_Draw(infoText, D3D_Xi(5), 60, D3D_D(5), COLOR_DGRAY, 1, \
			TEXT_CENTER); \
	}
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	Button_Draw(playButton, 0);
	Button_Draw(playSeqButton, 0);
	Button_Draw(editButton, 0);
	Button_Draw(copySwapButton, 0);
	Button_Draw(deleteButton, 0);
	Text_Draw(playText, PLAY_BUTTON_X + 24, BUTTON_Y + 5, 0.5, COLOR_LGRAY,
			1, TEXT_CENTER);
	Text_Draw(playSeqText, PLAYSEQ_BUTTON_X + 50, BUTTON_Y + 5, 0.5, COLOR_LGRAY,
			1, TEXT_CENTER);
	Text_Draw(editText, EDIT_BUTTON_X + 24, BUTTON_Y + 5, 0.5, COLOR_LGRAY,
			1, TEXT_CENTER);
	Text_Draw(copySwapText, COPY_BUTTON_X + 24, BUTTON_Y + 5, 0.5, COLOR_LGRAY,
			1, TEXT_CENTER);
	Text_Draw(deleteText, DELETE_BUTTON_X + 24, BUTTON_Y + 5, 0.5, COLOR_LGRAY,
			1, TEXT_CENTER);

	for (int i = 0; i < NUM_CARD_COLS; i++) {
		for (int j = 0; j < NUM_CARD_ROWS; j++) {
			float cardX = CARD_X_START + i*CARD_X_GAP;
			float cardY = CARD_Y_START + j*CARD_Y_GAP;
			Button_Draw(cards[i + j*NUM_CARD_COLS], 0);
			Text_Draw(cardNumbers[i + j*NUM_CARD_COLS],
					cardX + CARD_WIDTH/2, cardY + 10,
					0.5, COLOR_LGRAY, 1, TEXT_CENTER);
		}
	}

	if (selectedLevel >= 0) {
		int x = CARD_X_START + (selectedLevel % NUM_CARD_COLS) * CARD_X_GAP;
		int y = CARD_Y_START + (selectedLevel / NUM_CARD_COLS) * CARD_Y_GAP;
		drawOutline(x, y, 0.5, CARD_WIDTH, CARD_HEIGHT, COLOR_DRED, 2);
	}

	Animation_Draw(0.5);
}

Scene sceneLevelSelector = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

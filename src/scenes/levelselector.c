#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "levelselector.h"
#include "title.h"
#include "editor.h"
#include "components/button.h"
#include "components/text.h"
#include "../rendering/rendertarget.h"
#include "../rendering/colors.h"
#include "../rendering/spritesheet.h"
#include "../util/touchinput.h"
#include "../util/dispatcher.h"
#include "../levelio.h"

#define NUM_BUTTON_ROWS 6
#define NUM_BUTTON_COLUMNS 3

#define BUTTON_X_START 12
#define BUTTON_Y_START 12
#define BUTTON_GAP_X 50
#define BUTTON_GAP_Y 73

static Dispatcher touchDispatcher;
static Button buttons[NUM_BUTTON_ROWS][NUM_BUTTON_COLUMNS];

static Text nameText;

Scene_Params LevelSelector_MakeParams() {
	return (Scene_Params) {};
}

static void displayLevel() {
	// Since each button uses the same callback, we must infer which button it is
	TouchInput_Swipe swipe = TouchInput_GetSwipe();
	int buttonRow = (swipe.start.px - BUTTON_X_START) / BUTTON_GAP_X;
	int buttonCol = (swipe.start.py - BUTTON_Y_START) / BUTTON_GAP_Y;

	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(buttonRow + buttonCol * NUM_BUTTON_ROWS, false, path);
	LevelIO_Hole hole;	//TODO Maybe allow passing NULLs?
	LevelIO_Proj proj;
	Tile (*tiles)[LEVEL_HEIGHT_TILES];
	int width, par;
	char *name;

	if (!LevelIO_Read(path, &hole, &proj, &tiles, &width, &par, &name)) return;
	free(tiles);  // Don't need this

	Text_SetContent(nameText, "%s", name);
	free(name);
}

static bool sceneInit(Scene_Params ignored) {
	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	for (size_t r = 0; r < NUM_BUTTON_ROWS; r++) {
		for (size_t c = 0; c < NUM_BUTTON_COLUMNS; c++) {
			buttons[r][c] = Button_Create(
					BUTTON_X_START + (BUTTON_GAP_X) * r,
					BUTTON_Y_START + (BUTTON_GAP_Y) * c,
					SPRITE_LEVEL_SELECTOR_BUTTON,
					displayLevel
				);
			if (!buttons[r][c]) goto f_buttons;
			Button_RegisterForTouchEvents(buttons[r][c], touchDispatcher,
					0);
		}
	}

	nameText = Text_Create(EDITOR_LEVEL_NAME_MAX);
	if (!nameText) goto f_nameText;

	return true;

f_nameText:
f_buttons:
	for (size_t r = 0; r < NUM_BUTTON_ROWS; r++) {
		for (size_t c = 0; c < NUM_BUTTON_COLUMNS; c++) {
			if (buttons[r][c]) Button_Free(buttons[r][c]);
		}
	}
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	return false;
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
	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	Text_Draw(nameText, 10, 10, 0, COLOR_DGREEN, 1);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	for (size_t r = 0; r < NUM_BUTTON_ROWS; r++) {
		for (size_t c = 0; c < NUM_BUTTON_COLUMNS; c++) {
			Button_Draw(buttons[r][c], 0);
		}
	}
}

static void sceneExit() {}

Scene sceneLevelSelector = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

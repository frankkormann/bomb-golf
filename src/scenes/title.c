#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "title.h"
#include "course.h"
#include "editor.h"
#include "error.h"
#include "components/text.h"
#include "../savedata.h"
#include "../rendering/colors.h"
#include "../rendering/rendertarget.h"
#include "../util/macros.h"

#define TITLE_TEXT "Bomb Golf"
#define MIN_LEVEL_NUM 1
#define MAX_LEVEL_NUM (MIN_LEVEL_NUM - 1 + SAVEDATA_NUM_LEVELS)

typedef enum {
	START_ROMFS,
	START_SAVED,
	LEVEL_EDITOR,
	NUM_OPTIONS
} Title_Option;

static char *options[] = {
	"Start",
	"Start (Custom)",
	"Level Editor"
};

static C2D_Text titleText, cursorText, optionsText[NUM_OPTIONS];
static int cursor;

static C2D_TextBuf textBuf;

// Allow value to persist through scene init/exit cycles
static int levelNum = MIN_LEVEL_NUM;
static Text levelNumText;

Scene_Params Title_MakeParams() {
	return (Scene_Params) {};
}

static bool sceneInit(Scene_Params ignored) {
	char *errMsg = "";

	textBuf = C2D_TextBufNew(256);
	if (!textBuf) {
		errMsg = "Out of memory";
		goto f_textBuf;
	}

	levelNumText = Text_Create(8, NULL);
	if (!levelNumText) {
		errMsg = "Out of memory";
		goto f_levelNumText;
	}
	
	C2D_TextParse(&titleText, textBuf, TITLE_TEXT);
	C2D_TextParse(&cursorText, textBuf, "->");
	C2D_TextOptimize(&titleText);
	C2D_TextOptimize(&cursorText);

	for (size_t i = 0; i < NUM_OPTIONS; i++) {
		C2D_TextParse(&optionsText[i], textBuf, options[i]);
		C2D_TextOptimize(&optionsText[i]);
	}

	cursor = START_ROMFS;
	return true;

f_levelNumText:
	C2D_TextBufDelete(textBuf);
f_textBuf:
	Scene_SetNext(sceneError, Error_MakeParams(errMsg));
	return false;
}

static void sceneUpdate() {
	u32 kDown = hidKeysDown();

	if (kDown & KEY_UP) cursor--;
	if (kDown & KEY_DOWN) cursor++;
	cursor = clamp(cursor, 0, NUM_OPTIONS - 1);

	if (kDown & KEY_LEFT) levelNum--;
	if (kDown & KEY_RIGHT) levelNum++;
	levelNum = clamp(levelNum, MIN_LEVEL_NUM, MAX_LEVEL_NUM);

	if (kDown & KEY_A) {
		switch (cursor) {
			case START_ROMFS:
				Scene_SetNext(sceneCourse,
						Course_MakeParams(1, true));
				break;
			case START_SAVED:
				Scene_SetNext(sceneCourse,
						Course_MakeParams(levelNum, false));
				break;
			case LEVEL_EDITOR:
				Scene_SetNext(sceneEditor,
						Editor_MakeParams(levelNum));
				break;
			default:
				break;
		}
	}

	Text_SetContent(levelNumText, "%i / %i", levelNum, MAX_LEVEL_NUM);
}

static void sceneDraw() {
	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_WHITE);
	C2D_SceneBegin(top);

	C2D_DrawText(&titleText, 0, 160, 50, 0, 1, 1);
	C2D_DrawText(&cursorText, 0, 70, 100 + 15 * cursor, 0, 0.5, 0.5);
	for (size_t i = 0; i < NUM_OPTIONS; i++) {
		C2D_DrawText(&optionsText[i], 0, 100, 100 + 15 * i, 0, 0.5, 0.5);
	}

	if (cursor == START_SAVED || cursor == LEVEL_EDITOR) {
		C2D_DrawText(&levelNumText->text, 0, 270, 100 + 15 * cursor, 0, 0.5,
				0.5);
	}
	
	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_WHITE);
	C2D_SceneBegin(bottom);
}

static void sceneExit() {
	C2D_TextBufDelete(textBuf);
	Text_Free(levelNumText);
}

Scene sceneTitle = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

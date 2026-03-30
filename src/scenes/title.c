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
#include "../rendering/spritesheet.h"
#include "../util/macros.h"

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
	"Start [Custom]",
	"Level Editor"
};

static Text cursorText, optionsText[NUM_OPTIONS];
static int cursor;

// Allow value to persist through scene init/exit cycles
static int levelNum = MIN_LEVEL_NUM;
static Text levelNumText;

Scene_Params Title_MakeParams() {
	return (Scene_Params) {};
}

static bool sceneInit(Scene_Params ignored) {
	char *errMsg = "";

	cursorText = Text_Create(3);
	if (!cursorText) {
		errMsg = "Out of memory";
		goto f_cursorText;
	}
	Text_SetContent(cursorText, "->");

	levelNumText = Text_Create(10);
	if (!levelNumText) {
		errMsg = "Out of memory";
		goto f_levelNumText;
	}

	for (size_t i = 0; i < NUM_OPTIONS; i++) {
		optionsText[i] = Text_Create(strlen(options[i]) + 1);
		if (!optionsText[i]) {
			errMsg = "Out of memory";
			goto f_optionsText;
		}
		Text_SetContent(optionsText[i], options[i]);
	}

	cursor = START_ROMFS;
	return true;

f_optionsText:
	for (size_t i = 0; i < NUM_OPTIONS; i++) {
		if (optionsText[i]) Text_Free(optionsText[i]);
	}
	Text_Free(levelNumText);
f_levelNumText:
	Text_Free(cursorText);
f_cursorText:
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
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	SpriteSheet_Draw(SPRITE_TITLE, 200, 120, 1, 0, false, false);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_WHITE);
	C2D_SceneBegin(bottom);

	Text_Draw(cursorText, 70, 100 + TEXT_LINE_HEIGHT * cursor, 0, 1);
	for (size_t i = 0; i < NUM_OPTIONS; i++) {
		Text_Draw(optionsText[i], 100, 100 + TEXT_LINE_HEIGHT * i, 0, 1);
	}

	if (cursor == START_SAVED || cursor == LEVEL_EDITOR) {
		Text_Draw(levelNumText, 230, 100 + TEXT_LINE_HEIGHT * cursor, 0, 1);
	}
}

static void sceneExit() {
	Text_Free(cursorText);
	Text_Free(levelNumText);
	for (size_t i = 0; i < NUM_OPTIONS; i++) {
		Text_Free(optionsText[i]);
	}
}

Scene sceneTitle = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

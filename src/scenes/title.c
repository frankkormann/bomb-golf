#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "title.h"
#include "course.h"
#include "editor.h"
#include "../rendering/rendertarget.h"

#define TITLE_TEXT "Bomb Golf"

typedef enum {
	START_ROMFS,
	START_SDMC,
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

static int num = 1;

Scene_Params Title_MakeParams() {
	return (Scene_Params) {};
}

static bool sceneInit(Scene_Params ignored) {
	textBuf  = C2D_TextBufNew(256);
	if (!textBuf) return false;
	
	C2D_TextParse(&titleText, textBuf, TITLE_TEXT);
	C2D_TextParse(&cursorText, textBuf, "->");
	C2D_TextOptimize(&titleText);
	C2D_TextOptimize(&cursorText);

	for (size_t i = 0; i < NUM_OPTIONS; i++) {
		C2D_TextParse(&optionsText[i], textBuf, options[i]);
		C2D_TextOptimize(&optionsText[i]);
	}

	cursor = 0;

	return true;
}

static void sceneUpdate() {
	u32 kDown = hidKeysDown();

	if (kDown & KEY_UP) cursor--;
	if (kDown & KEY_DOWN) cursor++;

	if (cursor < 0) cursor = 0;
	if (cursor > NUM_OPTIONS-1) cursor = NUM_OPTIONS-1;

	if (kDown & KEY_A) {
		switch (cursor) {
			case START_ROMFS:
				Scene_SetNext(sceneCourse,
						Course_MakeParams(1, false));
				break;
			case START_SDMC:
				Scene_SetNext(sceneCourse,
						Course_MakeParams(num, true));
				break;
			case LEVEL_EDITOR:
				Scene_SetNext(sceneEditor, Editor_MakeParams(num));
				break;
			default:
				break;
		}
	}

	if (kDown & KEY_LEFT) num--;
	if (kDown & KEY_RIGHT) num++;
}

static void sceneDraw() {
	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, C2D_Color32(255, 255, 255, 255));
	C2D_SceneBegin(top);

	C2D_DrawText(&titleText, 0, 160, 50, 0, 1, 1);
	C2D_DrawText(&cursorText, 0, 70, 100 + 15 * cursor, 0, 0.5, 0.5);
	for (size_t i = 0; i < NUM_OPTIONS; i++) {
		C2D_DrawText(&optionsText[i], 0, 100, 100 + 15 * i, 0, 0.5, 0.5);
	}

	if (cursor == START_SDMC || cursor == LEVEL_EDITOR) {
		for (int i = 0; i < num; i++) {
			C2D_DrawRectSolid(200 + 15 * i, 100 + 15 * cursor, 0, 10, 10,
					C2D_Color32(0, 200, 120, 255));
		}
	}
	
	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, C2D_Color32(255, 255, 255, 255));
	C2D_SceneBegin(bottom);
}

static void sceneExit() {
	C2D_TextBufDelete(textBuf);
}

Scene sceneTitle = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

#include <stdbool.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "title.h"
#include "course.h"
#include "levelselector.h"
#include "error.h"
#include "components/button.h"
#include "components/text.h"
#include "../rendering/colors.h"
#include "../rendering/rendertarget.h"
#include "../rendering/spritesheet.h"
#include "../util/dispatcher.h"

#define BUTTON_X 60
#define BUTTON_START_Y 45
#define BUTTON_EDITOR_Y 135

static Text   startText,   editorText;
static Button startButton, editorButton;

static Dispatcher touchDispatcher;

Scene_Params Title_MakeParams() {
	return (Scene_Params) {};
}

static void startGame(void *ignored) {
	Scene_SetNext(sceneCourse, Course_MakeParams(1, true));
}

static void openEditor(void *ignored) {
	Scene_SetNext(sceneLevelSelector, LevelSelector_MakeParams(-1));
}

static bool sceneInit(Scene_Params ignored) {
	startText = Text_Create(8);
	if (!startText) goto f_startText;
	Text_SetContent(startText, "Start");

	editorText = Text_Create(16);
	if (!editorText) goto f_editorText;
	Text_SetContent(editorText, "Level Editor");

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	startButton = Button_Create(BUTTON_X, BUTTON_START_Y, SPRITE_LARGE_BUTTON,
			NULL, startGame);
	if (!startButton) goto f_startButton;
	Button_RegisterForTouchEvents(startButton, touchDispatcher, 1);

	editorButton = Button_Create(BUTTON_X, BUTTON_EDITOR_Y, SPRITE_LARGE_BUTTON,
			NULL, openEditor);
	if (!editorButton) goto f_editorButton;
	Button_RegisterForTouchEvents(editorButton, touchDispatcher, 1);

	return true;

f_editorButton:
	Button_Free(startButton);
f_startButton:
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	Text_Free(editorText);
f_editorText:
	Text_Free(startText);
f_startText:
	Scene_SetNext(sceneError, Error_MakeParams("Out of memory"));
	return false;
}

static void sceneUpdate() {
	Dispatcher_DispatchEvent(touchDispatcher);
}

static void sceneDraw() {
	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	SpriteSheet_Draw(SPRITE_TITLE, 0, 0, 1, 0, false, false);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	Button_Draw(startButton, 0);
	Text_Draw(startText, BUTTON_X + 20, BUTTON_START_Y + 10, 0, COLOR_LGRAY, 2);
	Button_Draw(editorButton, 0);
	Text_Draw(editorText, BUTTON_X +20, BUTTON_EDITOR_Y + 10, 0, COLOR_LGRAY, 2);
}

static void sceneExit() {
	Text_Free(startText);
	Text_Free(editorText);
	Button_Free(startButton);
	Button_Free(editorButton);
}

Scene sceneTitle = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

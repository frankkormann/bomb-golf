#include <stdbool.h>
#include <limits.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "title.h"
#include "arsenal.h"
#include "levelselector.h"
#include "error.h"
#include "settings.h"
#include "components/button.h"
#include "components/text.h"
#include "../rendering/color.h"
#include "../rendering/rendertarget.h"
#include "../rendering/spritesheet.h"
#include "../rendering/animation.h"
#include "../rendering/draw3d.h"
#include "../file/saveio.h"
#include "../util/dispatcher.h"
#include "../util/tracker.h"

#define TROPHY_X	20
#define TROPHY_Y	180
#define DOVE_X		(400 - 48 - 20)
#define DOVE_Y		(240 - 36 - 15)
#define GRAVE_X		(DOVE_X - 32 - 10)
#define GRAVE_Y		(240 - 40 - 13)
#define BEST_SCORE_X	390
#define BEST_SCORE_Y	10

#define BUTTON_X	58
#define START_Y		45
#define EDITOR_Y	(START_Y + 70)
#define SETTINGS_Y	(EDITOR_Y + 35)

static Button startButton, editorButton, practiceButton, settingsButton;
static Text   startText,   editorText,   practiceText,   settingsText, hiscoreText;
static Dispatcher touchDispatcher, keyDispatcher;
static SaveIO_Achvmt achievements;

static void startGame() {
	Tracker_Clear();
	Scene_Switch(sceneArsenal, &(Arsenal_Params) SCENE_PARAMS_EMPTY);
}

static void openEditor() {
	Scene_Switch(sceneLevelSelector, &(LevelSelector_Params) { -1, false });
}

static void openPractice() {
	Scene_Switch(sceneLevelSelector, &(LevelSelector_Params) { -1, true });
}

static void openSettings() {
	Scene_Switch(sceneSettings, &(Settings_Params) SCENE_PARAMS_EMPTY);
}

static bool sceneInit() {
	startText = Text_Create(6);
	if (!startText) goto f_startText;
	Text_SetContent(startText, "Start");

	editorText = Text_Create(13);
	if (!editorText) goto f_editorText;
	Text_SetContent(editorText, "Level Editor");

	practiceText = Text_Create(9);
	if (!practiceText) goto f_practiceText;
	Text_SetContent(practiceText, "Practice");

	settingsText = Text_Create(9);
	if (!settingsText) goto f_settingsText;
	Text_SetContent(settingsText, "Settings");

	int hiscore;
	if (SaveIO_ReadOverallHighScore(&hiscore) && hiscore != INT_MAX) {
		hiscoreText = Text_Create(16);
		if (!hiscoreText) goto f_hiscoreText;
		Text_SetContent(hiscoreText, "Best Score: %+i", hiscore);
	} else {
		hiscoreText = NULL;
	}

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	keyDispatcher = Dispatcher_Create();
	if (!keyDispatcher) goto f_keyDispatcher;

	startButton = Button_Create(BUTTON_X, START_Y, SPRITE_LARGE_BUTTON, KEY_A,
			NULL, startGame);
	if (!startButton) goto f_startButton;
	Button_RegisterForTouchEvents(startButton, touchDispatcher, 1);
	Button_RegisterForKeyEvents(startButton, keyDispatcher, 1);

	editorButton = Button_Create(BUTTON_X, EDITOR_Y, SPRITE_MEDIUM_BUTTON, KEY_X,
			NULL, openEditor);
	if (!editorButton) goto f_editorButton;
	Button_RegisterForTouchEvents(editorButton, touchDispatcher, 1);
	Button_RegisterForKeyEvents(editorButton, keyDispatcher, 1);

	practiceButton = Button_Create(BUTTON_X + 104, EDITOR_Y,
			SPRITE_MEDIUM_BUTTON, KEY_Y, NULL, openPractice);
	if (!practiceButton) goto f_practiceButton;
	Button_RegisterForTouchEvents(practiceButton, touchDispatcher, 1);
	Button_RegisterForKeyEvents(practiceButton, keyDispatcher, 1);

	settingsButton = Button_Create(BUTTON_X, SETTINGS_Y, SPRITE_LONG_BUTTON,
			-1, NULL, openSettings);
	if (!settingsButton) goto f_settingsButton;
	Button_RegisterForTouchEvents(settingsButton, touchDispatcher, 1);

	if (!SaveIO_ReadAchievements(&achievements)) {
		achievements = 0;
	}
	return true;

f_settingsButton:
	Button_Free(practiceButton);
f_practiceButton:
	Button_Free(editorButton);
f_editorButton:
	Button_Free(startButton);
f_startButton:
	Dispatcher_Free(keyDispatcher);
f_keyDispatcher:
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	if (hiscoreText) Text_Free(hiscoreText);
f_hiscoreText:
	Text_Free(settingsText);
f_settingsText:
	Text_Free(practiceText);
f_practiceText:
	Text_Free(editorText);
f_editorText:
	Text_Free(startText);
f_startText:
	Scene_Switch(sceneError, &(Error_Params) { "Out of memory" });
	return false;
}

static void sceneExit() {
	Text_Free(startText);
	Text_Free(editorText);
	Text_Free(settingsText);
	Text_Free(practiceText);
	if (hiscoreText) Text_Free(hiscoreText);
	Button_Free(startButton);
	Button_Free(editorButton);
	Button_Free(settingsButton);
	Dispatcher_Free(touchDispatcher);
	Dispatcher_Free(keyDispatcher);
	Button_Free(practiceButton);
}

static void sceneUpdate(float _) {
	Dispatcher_DispatchEvent(touchDispatcher);
	Dispatcher_DispatchEvent(keyDispatcher);
}

static void sceneDraw() {
	#define D3D_VALS { \
			{ BEST_SCORE_X, 0.6 }, \
			{ TROPHY_X, 0.6 }, \
			{ DOVE_X, 0.6 }, \
			{ GRAVE_X, 0.6 }, \
			{ 0, 0.8 } \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, COLOR_LGRAY); \
	C2D_SceneBegin(D3D_TARGET); \
	\
	if (hiscoreText) { \
		Text_Draw(hiscoreText, D3D_Xi(0), BEST_SCORE_Y, D3D_D(0), \
				COLOR_DGRAY, 1, TEXT_RIGHT); \
	} \
	SpriteSheet_Sprite trophySpr = 0; \
	if (achievements & ACHVMT_SCORE_OK) trophySpr = SPRITE_TROPHY_OK; \
	if (achievements & ACHVMT_SCORE_GOOD) trophySpr = SPRITE_TROPHY_GOOD; \
	if (achievements & ACHVMT_SCORE_GREAT) trophySpr = SPRITE_TROPHY_GREAT; \
	if (trophySpr != 0) { \
		SpriteSheet_Draw(trophySpr, D3D_Xi(1), TROPHY_Y, D3D_D(1), 0, \
				false, false); \
	} \
	if (achievements & ACHVMT_PEACE) { \
		SpriteSheet_Draw(SPRITE_TROPHY_DOVE, D3D_Xi(2), DOVE_Y, D3D_D(2), \
				0, false, false); \
	} \
	if (achievements & ACHVMT_GENOCIDE) { \
		SpriteSheet_Draw(SPRITE_TROPHY_GRAVE, D3D_Xi(3), GRAVE_Y, D3D_D(3), \
				0, false, false); \
	} \
	SpriteSheet_Draw(SPRITE_TITLE, D3D_Xi(4), 0, D3D_D(4), 0, false, false);
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	Button_Draw(startButton, 0);
	Text_Draw(startText, BUTTON_X + 15, START_Y + 10, 0, COLOR_LGRAY, 2,
			TEXT_LEFT);
	Button_Draw(editorButton, 0);
	Text_Draw(editorText, BUTTON_X + 15, EDITOR_Y + 5, 0, COLOR_LGRAY, 1,
			TEXT_LEFT);
	Button_Draw(practiceButton, 0);
	Text_Draw(practiceText, BUTTON_X + 119, EDITOR_Y + 5, 0, COLOR_LGRAY, 1,
			TEXT_LEFT);
	Button_Draw(settingsButton, 0);
	Text_Draw(settingsText, BUTTON_X + 15, SETTINGS_Y + 5, 0, COLOR_LGRAY, 1,
			TEXT_LEFT);

	Animation_Draw(0.5);
}

Scene sceneTitle = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

#include <stdlib.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "results.h"
#include "error.h"
#include "title.h"
#include "levelselector.h"
#include "components/text.h"
#include "components/button.h"
#include "components/border.h"
#include "components/tracer.h"
#include "../rendering/rendertarget.h"
#include "../rendering/colors.h"
#include "../util/dispatcher.h"
#include "../terrain.h"
#include "../levelio.h"
#include "../savedata.h"

#define TEXT_MARGIN 10
#define COMPLETE_TEXT_Y 20
#define LEVEL_NAME_Y /* TODO */
#define LEVEL_STATS_Y (COMPLETE_TEXT_Y + 2*TEXT_LINE_HEIGHT)

#define TIMER_REVEAL_PAR                           30
#define TIMER_REVEAL_STROKES (TIMER_REVEAL_PAR   + 45)
#define TIMER_REVEAL_SCORE (TIMER_REVEAL_STROKES + 45)

#define LEVEL_PREVIEW_X 10
#define LEVEL_PREVIEW_Y (LEVEL_STATS_Y + 50)
#define LEVEL_PREVIEW_WIDTH 380
#define LEVEL_PREVIEW_HEIGHT (240 - 35 - LEVEL_PREVIEW_Y)

#define BUTTON_X 60
#define BUTTON_START_Y 45
#define BUTTON_GAP 90

static int level, nextLevel;
static bool levelInRomfs;

static int score;

static Text completeText, nameText, parText, strokesText, scoreText;
static int textRevealCounter;
static Tracer projPath;

static Text   nextText,   quitText;
static Button nextButton, quitButton;
static Dispatcher touchDispatcher;

Scene_Params Results_MakeParams(int strokes, int level, bool levelInRomfs,
		Tracer projPath) {
	return (Scene_Params) { .results = {
		.strokes = strokes,
		.level = level,
		.levelInRomfs = levelInRomfs,
		.projPath = projPath
	} };
}

static void getScoreForStrokes(int score, char *buf) {
	switch (score) {
		case -4: 
			strcpy(buf, "Condor");
			return;
		case -3:
			strcpy(buf, "Albatross");
			return;
		case -2:
			strcpy(buf, "Eagle");
			return;
		case -1:
			strcpy(buf, "Birdie");
			return;
		case 0:
			strcpy(buf, "Par");
			return;
		case 1:
			strcpy(buf, "Bogey");
			return;
		case 2:
			strcpy(buf, "Double Bogey");
			return;
		case 3:
			strcpy(buf, "Triple Bogey");
			return;
		case 4:
			strcpy(buf, "Quadruple Bogey");
			return;
		case 5:
			strcpy(buf, "Quintuple Bogey");
			return;
		case 6:
			strcpy(buf, "Sextuple Bogey");
			return;
		case 7:
			strcpy(buf, "Septuple Bogey");
			return;
		case 8:
			strcpy(buf, "Octuple Bogey");
			return;
		case 9:
			strcpy(buf, "Nonuple Bogey");
			return;
	}
	if (score > 0) {
		sprintf(buf, "%i Over Par", score);
	} else {
		sprintf(buf, "%i Under Par", -score);
	}
}

static void goNextLevel() {
	Scene_SetNext(sceneCourse, Course_MakeParams(nextLevel, levelInRomfs));
}

static void quit() {
	if (levelInRomfs) {
		Scene_SetNext(sceneTitle, Title_MakeParams());
	} else {
		Scene_SetNext(sceneLevelSelector, LevelSelector_MakeParams(level));
	}
}

static bool sceneInit(Scene_Params params) {
	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(params.results.level, params.results.levelInRomfs, path);
	int par;
	char *name;
	if (!LevelIO_Read(path, NULL, NULL, NULL, NULL, NULL, NULL, &par, &name)) {
		goto f_LevelIO_Read;
	}

	completeText = Text_Create(strlen("Hole Complete!") + 1);
	if (!completeText) goto f_completeText;
	Text_SetContent(completeText, "Hole Complete!");

	nameText = Text_Create(strlen(name) + 1);
	if (!nameText) goto f_nameText;
	Text_SetContent(nameText, name);

	parText = Text_Create(16);
	if (!parText) goto f_parText;
	Text_SetContent(parText, "Par %i", par);

	strokesText = Text_Create(16);
	if (!strokesText) goto f_strokesText;
	Text_SetContent(strokesText, "Strokes %i", params.results.strokes);

	scoreText = Text_Create(16);
	if (!scoreText) goto f_scoreText;
	char buf[32];
	getScoreForStrokes(params.results.strokes - par, buf);
	Text_SetContent(scoreText, buf);

	nextText = Text_Create(16);
	if (!nextText) goto f_nextText;
	Text_SetContent(nextText, "Next Hole");

	quitText = Text_Create(8);
	if (!quitText) goto f_quitText;
	Text_SetContent(quitText, "Exit");

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	nextButton = Button_Create(BUTTON_X, BUTTON_START_Y, SPRITE_LARGE_BUTTON,
			NULL, goNextLevel);
	if (!nextButton) goto f_nextButton;
	Button_RegisterForTouchEvents(nextButton, touchDispatcher, 1);

	quitButton = Button_Create(BUTTON_X, BUTTON_START_Y + BUTTON_GAP,
			SPRITE_LARGE_BUTTON, NULL, quit);
	if (!quitButton) goto f_quitButton;
	Button_RegisterForTouchEvents(quitButton, touchDispatcher, 1);

	//TODO Create a "final results"/summary Scene
	nextLevel = params.results.level + 1;
	while (true) {
		char path[LEVEL_PATH_MAX];
		LevelIO_MakePath(nextLevel, params.results.levelInRomfs, path);
		if (FILE *f = fopen(path, "rb")) {
			fclose(f);
			break;
		}
		if (nextLevel >= SAVEDATA_NUM_LEVELS) {
			nextLevel = -1;
			Button_Disable(nextButton);
			break;
		}
		nextLevel++;
	}

	level = params.results.level;
	levelInRomfs = params.results.levelInRomfs;
	score = params.results.strokes - par;

	textRevealCounter = 0;
	projPath = params.results.projPath;

	return true;

f_quitButton:
	Button_Free(nextButton);
f_nextButton:
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	Text_Free(quitText);
f_quitText:
	Text_Free(nextText);
f_nextText:
	Text_Free(scoreText);
f_scoreText:
	Text_Free(strokesText);
f_strokesText:
	Text_Free(parText);
f_parText:
	Text_Free(nameText);
f_nameText:
	Text_Free(completeText);
f_completeText:
f_LevelIO_Read:
	Scene_SetNext(sceneError, Error_MakeParams("Out of memory"));
	return false;
}

static void sceneExit() {
	Text_Free(completeText);
	Text_Free(nameText);
	Text_Free(parText);
	Text_Free(strokesText);
	Text_Free(scoreText);
	Tracer_Free(projPath);
	Text_Free(nextText);
	Button_Free(nextButton);
	Text_Free(quitText);
	Button_Free(quitButton);
	Dispatcher_Free(touchDispatcher);
	Terrain_Exit();
}

static void sceneUpdate() {
	u32 kDown = hidKeysDown();

	//TODO Figure out a better way to provide keybinds for all Buttons
	if (kDown & KEY_B) {
		if (levelInRomfs) {
			Scene_SetNext(sceneTitle, Title_MakeParams());
		} else {
			Scene_SetNext(sceneLevelSelector,
					LevelSelector_MakeParams(level));
		}
	}
	if (kDown & KEY_A) {
		Scene_SetNext(sceneCourse, Course_MakeParams(nextLevel,
				levelInRomfs));
	}

	Dispatcher_DispatchEvent(touchDispatcher);

	textRevealCounter++;
}

static u32 getColorForScore(int score) {
	if (score <= -2) {
		return COLOR_YELLOW;
	} else if (score <= -1) {
		return COLOR_BLUE;
	} else if (score <= 0) {
		return COLOR_GREEN;
	} else {
		return COLOR_DBROWN;
	}
}

static void sceneDraw() {
	Tracer_UpdateGraphics(projPath);


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	Text_Draw(completeText, 200, COMPLETE_TEXT_Y, 0, COLOR_DGREEN, 2,
			TEXT_CENTERED);
//	Text_Draw(nameText, 200, LEVEL_NAME_Y, 0, COLOR_DGREEN, 1, TEXT_CENTERED);
	if (textRevealCounter >= TIMER_REVEAL_PAR) {
		Text_Draw(parText, 150, LEVEL_STATS_Y, 0, COLOR_DGRAY, 1,
				TEXT_CENTERED);
	}
	if (textRevealCounter >= TIMER_REVEAL_STROKES) {
		Text_Draw(strokesText, 250, LEVEL_STATS_Y, 0, COLOR_DGRAY, 1,
				TEXT_CENTERED);
	}
	if (textRevealCounter >= TIMER_REVEAL_SCORE) {
		Text_Draw(scoreText, 200, LEVEL_STATS_Y + TEXT_LINE_HEIGHT, 0,
				getColorForScore(score), 1, TEXT_CENTERED);
	}

	int terrainX, terrainY, terrainWidth, terrainHeight;
	Terrain_Draw(LEVEL_PREVIEW_X, LEVEL_PREVIEW_Y, 0, LEVEL_PREVIEW_WIDTH,
			LEVEL_PREVIEW_HEIGHT, &terrainX, &terrainY, &terrainWidth,
			&terrainHeight);
	Border_Draw(terrainX, terrainY, 0, terrainWidth, terrainHeight);
	Tracer_Draw(projPath, terrainX, terrainY, 0.5, terrainWidth, terrainHeight);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	Button_Draw(nextButton, 0.5);
	Text_Draw(nextText, BUTTON_X + 20, BUTTON_START_Y + 10, 1, COLOR_LGRAY, 2,
			TEXT_LEFT);
	Button_Draw(quitButton, 0.5);
	Text_Draw(quitText, BUTTON_X + 20, BUTTON_START_Y + BUTTON_GAP + 10, 1,
			COLOR_LGRAY, 2, TEXT_LEFT);
}

Scene sceneResults = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

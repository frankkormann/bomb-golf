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
#include "../environment/terrain.h"
#include "../rendering/rendertarget.h"
#include "../rendering/colors.h"
#include "../rendering/animation.h"
#include "../rendering/draw3d.h"
#include "../audio/music.h"
#include "../util/dispatcher.h"
#include "../util/tracker.h"
#include "../levelio.h"
#include "../savedata.h"

#define COMPLETE_TEXT_Y		20
#define LEVEL_PREVIEW_X		10
#define LEVEL_PREVIEW_Y		(COMPLETE_TEXT_Y + 20 + 2*TEXT_LINE_HEIGHT)
#define LEVEL_PREVIEW_WIDTH	380
#define LEVEL_PREVIEW_HEIGHT	(240 - 35 - LEVEL_PREVIEW_Y)

#define NUM_TEXT_GAP		80
#define PAR_TEXT_X		(BUTTON_X + 2)
#define PAR_TEXT_Y		50
#define STROKES_TEXT_X		PAR_TEXT_X
#define STROKES_TEXT_Y		(PAR_TEXT_Y + TEXT_LINE_HEIGHT)
#define SCORE_TEXT_Y		(STROKES_TEXT_Y + TEXT_LINE_HEIGHT + 10)
#define OVERALL_TEXT_X		(PAR_TEXT_X + NUM_TEXT_GAP + 23)
#define OVERALL_TEXT_Y		PAR_TEXT_Y
#define BUTTON_X		60
#define BUTTON_Y		(SCORE_TEXT_Y + TEXT_LINE_HEIGHT + 15)

#define TIMER_REVEAL_PAR	                        15
#define TIMER_REVEAL_STROKES	(TIMER_REVEAL_PAR     + 30)
#define TIMER_REVEAL_SCORE	(TIMER_REVEAL_STROKES + 30)

static int level, nextLevel;
static bool levelInRomfs;

static int strokes, par;

static Text completeText, parText, parNumText, strokesText, strokesNumText,
		scoreNameText, scoreTotText, scoreTotNumText;
static int textRevealCounter;
static Tracer projPath;

static Text   buttonText;
static Button nextButton, quitButton;
static Dispatcher touchDispatcher, keyDispatcher;

Scene_Params Results_MakeParams(int strokes, int level, bool levelInRomfs,
		Tracer projPath) {
	return (Scene_Params) { .results = {
		.strokes = strokes,
		.level = level,
		.levelInRomfs = levelInRomfs,
		.projPath = projPath
	} };
}

static void getScoreForStrokes(int strokes, int par, char *buf) {
	if (strokes == 1) {
		strcpy(buf, "Hole in One!");
		return;
	}

	switch (strokes - par) {
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
	if (strokes > par) {
		sprintf(buf, "%i Over Par", strokes - par);
	} else {
		sprintf(buf, "%i Under Par", par - strokes);
	}
}

static u32 getColorForScore(int strokes, int par) {
	//TODO Decide on better colors?
	if (strokes == 1) return COLOR_WHITE;

	int score = strokes - par;
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
	if (!LevelIO_Read(path, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &par,
			NULL, NULL)) {
		goto f_LevelIO_Read;
	}

	completeText = Text_Create(strlen("Hole Complete!") + 1);
	if (!completeText) goto f_completeText;
	Text_SetContent(completeText, "Hole Complete!");

	parText = Text_Create(4);
	if (!parText) goto f_parText;
	Text_SetContent(parText, "Par");

	parNumText = Text_Create(4);
	if (!parNumText) goto f_parNumText;
	Text_SetContent(parNumText, "%i", par);

	strokesText = Text_Create(8);
	if (!strokesText) goto f_strokesText;
	Text_SetContent(strokesText, "Strokes");

	strokesNumText = Text_Create(4);
	if (!strokesNumText) goto f_strokesNumText;
	Text_SetContent(strokesNumText, "%i", params.results.strokes);

	scoreNameText = Text_Create(16);
	if (!scoreNameText) goto f_scoreNameText;
	{
		char buf[32];
		getScoreForStrokes(params.results.strokes, par, buf);
		Text_SetContent(scoreNameText, buf);
	}

	scoreTotText = Text_Create(11);
	if (!scoreTotText) goto f_scoreTotText;
	Text_SetContent(scoreTotText, "Tot. Score");

	scoreTotNumText = Text_Create(4);
	if (!scoreTotNumText) goto f_scoreTotNumText;
	{
		int overall = 0;
		for (Tracker_Stat i = TRACKER_LVL1; i <= TRACKER_LVL18; i++) {
			overall += Tracker_Get(i);
		}
		Text_SetContent(scoreTotNumText, "%i", overall);
	}

	buttonText = Text_Create(16);
	if (!buttonText) goto f_buttonText;

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	keyDispatcher = Dispatcher_Create();
	if (!keyDispatcher) goto f_keyDispatcher;

	nextButton = Button_Create(BUTTON_X, BUTTON_Y,
			SPRITE_LARGE_BUTTON, KEY_A, NULL, goNextLevel);
	if (!nextButton) goto f_nextButton;
	Button_RegisterForTouchEvents(nextButton, touchDispatcher, 1);
	Button_RegisterForKeyEvents(nextButton, keyDispatcher, 1);
	Button_Disable(nextButton);

	quitButton = Button_Create(BUTTON_X, BUTTON_Y,
			SPRITE_LARGE_BUTTON, KEY_A | KEY_B, NULL, quit);
	if (!quitButton) goto f_quitButton;
	Button_RegisterForTouchEvents(quitButton, touchDispatcher, 1);
	Button_RegisterForKeyEvents(quitButton, keyDispatcher, 1);
	Button_Disable(quitButton);

	if (params.results.levelInRomfs) {
		//TODO Create a "final results"/summary Scene
		nextLevel = params.results.level + 1;
		while (true) {
			char path[LEVEL_PATH_MAX];
			LevelIO_MakePath(nextLevel, params.results.levelInRomfs,
					path);
			if (FILE *f = fopen(path, "rb")) {
				fclose(f);
				Text_SetContent(buttonText, "Next Hole");
				Button_Enable(nextButton);
				break;
			}
			if (nextLevel >= SAVEDATA_NUM_LEVELS) {
				nextLevel = -1;
				Text_SetContent(buttonText, "Back");
				Button_Enable(quitButton);
				break;
			}
			nextLevel++;
		}
	} else {
		Text_SetContent(buttonText, "Back");
		Button_Enable(quitButton);
	}

	Music_Start(MUSIC_RESULTS);

	level = params.results.level;
	levelInRomfs = params.results.levelInRomfs;
	strokes = params.results.strokes;

	textRevealCounter = 0;
	projPath = params.results.projPath;

	return true;

f_quitButton:
	Button_Free(nextButton);
f_nextButton:
	Dispatcher_Free(keyDispatcher);
f_keyDispatcher:
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	Text_Free(buttonText);
f_buttonText:
	Text_Free(scoreTotNumText);
f_scoreTotNumText:
	Text_Free(scoreTotText);
f_scoreTotText:
	Text_Free(scoreNameText);
f_scoreNameText:
	Text_Free(strokesNumText);
f_strokesNumText:
	Text_Free(strokesText);
f_strokesText:
	Text_Free(parNumText);
f_parNumText:
	Text_Free(parText);
f_parText:
	Text_Free(completeText);
f_completeText:
f_LevelIO_Read:
	Scene_SetNext(sceneError, Error_MakeParams("Out of memory"));
	return false;
}

static void sceneExit() {
	Text_Free(completeText);
	Text_Free(parText);
	Text_Free(parNumText);
	Text_Free(strokesText);
	Text_Free(strokesNumText);
	Text_Free(scoreNameText);
	Text_Free(scoreTotText);
	Text_Free(scoreTotNumText);
	Tracer_Free(projPath);
	Text_Free(buttonText);
	Button_Free(nextButton);
	Button_Free(quitButton);
	Dispatcher_Free(touchDispatcher);
	Dispatcher_Free(keyDispatcher);
	Terrain_Exit();
	Music_Stop();
}

static void sceneUpdate(float _) {
	Dispatcher_DispatchEvent(touchDispatcher);
	Dispatcher_DispatchEvent(keyDispatcher);

	textRevealCounter++;
}

static void sceneDraw() {
	Tracer_UpdateGraphics(projPath);


	int terrainX, terrainY, terrainWidth, terrainHeight;
	#define D3D_DEPTHS { 0.5, 0, 0.5, 0.5 }
	#define D3D_XS { \
			200, \
			LEVEL_PREVIEW_X, \
			terrainX + D3D_CORRECTION(2), \
			terrainX \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, COLOR_LGRAY); \
	C2D_SceneBegin(D3D_TARGET); \
	\
	Text_Draw(completeText, D3D_X(0), COMPLETE_TEXT_Y, D3D_D(0), COLOR_DGREEN, \
			2, TEXT_CENTERED); \
	\
	Terrain_Draw(D3D_X(1), LEVEL_PREVIEW_Y, D3D_D(1), LEVEL_PREVIEW_WIDTH, \
			LEVEL_PREVIEW_HEIGHT, &terrainX, &terrainY, &terrainWidth, \
			&terrainHeight); \
	Border_Draw(D3D_X(2), terrainY, D3D_D(2), \
			terrainWidth - 2*D3D_CORRECTION(2), terrainHeight); \
	Tracer_Draw(projPath, D3D_X(3), terrainY, D3D_D(3), terrainWidth, \
			terrainHeight);
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	if (textRevealCounter >= TIMER_REVEAL_PAR) {
		Text_Draw(parText, PAR_TEXT_X, PAR_TEXT_Y, 0, COLOR_DGRAY, 1,
				TEXT_LEFT);
		Text_Draw(parNumText, PAR_TEXT_X + NUM_TEXT_GAP, PAR_TEXT_Y, 0,
				COLOR_DGRAY, 1, TEXT_RIGHT);
	}
	if (textRevealCounter >= TIMER_REVEAL_STROKES) {
		Text_Draw(strokesText, STROKES_TEXT_X, STROKES_TEXT_Y, 0,
				COLOR_DGRAY, 1, TEXT_LEFT);
		Text_Draw(strokesNumText, STROKES_TEXT_X + NUM_TEXT_GAP,
				STROKES_TEXT_Y, 0, COLOR_DGRAY, 1, TEXT_RIGHT);
	}
	if (textRevealCounter >= TIMER_REVEAL_SCORE) {
		Text_Draw(scoreTotText, OVERALL_TEXT_X, OVERALL_TEXT_Y, 0,
				COLOR_DGRAY, 1, TEXT_LEFT);
		Text_Draw(scoreTotNumText, OVERALL_TEXT_X + NUM_TEXT_GAP + 15,
				OVERALL_TEXT_Y, 0, COLOR_DGRAY, 1, TEXT_RIGHT);
		Text_Draw(scoreNameText, 160, SCORE_TEXT_Y, 0,
				getColorForScore(strokes, par), 1, TEXT_CENTERED);
	}

	Button_Draw(nextButton, 0.5);
	Button_Draw(quitButton, 0.5);
	Text_Draw(buttonText, BUTTON_X + 20, BUTTON_Y + 10, 1, COLOR_LGRAY,
			2, TEXT_LEFT);

	Animation_Draw(0.5);
}

Scene sceneResults = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

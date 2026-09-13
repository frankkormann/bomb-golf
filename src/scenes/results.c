#include <stdlib.h>
#include <math.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "results.h"
#include "error.h"
#include "summary.h"
#include "levelselector.h"
#include "course.h"
#include "editor.h"
#include "components/text.h"
#include "components/border.h"
#include "components/tracer.h"
#include "../environment/terrain.h"
#include "../rendering/rendertarget.h"
#include "../rendering/color.h"
#include "../rendering/animation.h"
#include "../rendering/draw3d.h"
#include "../audio/music.h"
#include "../audio/soundeffect.h"
#include "../util/tracker.h"
#include "../util/touchinput.h"
#include "../util/macros.h"
#include "../levelio.h"
#include "../savedata.h"

#define COMPLETE_TEXT_Y		20
#define LEVEL_PREVIEW_X		10
#define LEVEL_PREVIEW_Y		(COMPLETE_TEXT_Y + 20 + 2*TEXT_LINE_HEIGHT)
#define LEVEL_PREVIEW_WIDTH	380
#define LEVEL_PREVIEW_HEIGHT	(240 - 35 - LEVEL_PREVIEW_Y)

#define NUM_TEXT_GAP		200
#define NAME_TEXT_Y		60
#define PAR_TEXT_X		60
#define PAR_TEXT_Y		(NAME_TEXT_Y + TEXT_LINE_HEIGHT)
#define STROKES_TEXT_X		PAR_TEXT_X
#define STROKES_TEXT_Y		(PAR_TEXT_Y + TEXT_LINE_HEIGHT)
#define SCORE_TEXT_Y		(STROKES_TEXT_Y + TEXT_LINE_HEIGHT + 5)
#define OVERALL_TEXT_X		PAR_TEXT_X
#define OVERALL_TEXT_Y		(SCORE_TEXT_Y + TEXT_LINE_HEIGHT + 20)

#define BOX_X			(PAR_TEXT_X - 10)
#define BOX_Y			(NAME_TEXT_Y - 5)
#define BOX_WIDTH		(NUM_TEXT_GAP + 20)
#define BOX_HEIGHT		(4*TEXT_LINE_HEIGHT + 15)

#define TIMER_REVEAL_PAR	                        15
#define TIMER_REVEAL_STROKES	(TIMER_REVEAL_PAR     + 30)
#define TIMER_REVEAL_SCORE	(TIMER_REVEAL_STROKES + 30)
#define TIMER_MAX		(TIMER_REVEAL_SCORE + 1)

static int level, nextLevel;
static bool levelInRomfs, isSequence;

static int strokes, par;

static Text completeText, parText, parNumText, strokesText, strokesNumText,
		scoreNameText, scoreTotText, scoreTotNumText, nameText;
static int textRevealCounter;
static Tracer projPath;

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

static bool sceneInit(void *sceneParams) {
	Results_Params *params = (Results_Params*)sceneParams;

	char path[LEVEL_PATH_MAX], *name;
	LevelIO_MakePath(params->level, params->levelInRomfs, path);
	if (!LevelIO_Read(path, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
			&par, &name, NULL)) {
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
	Text_SetContent(strokesNumText, "%i", params->strokes);

	scoreNameText = Text_Create(16);
	if (!scoreNameText) goto f_scoreNameText;
	{
		char buf[32];
		getScoreForStrokes(params->strokes, par, buf);
		Text_SetContent(scoreNameText, buf);
	}

	scoreTotText = Text_Create(16);
	if (!scoreTotText) goto f_scoreTotText;
	Text_SetContent(scoreTotText, "Overall Score");

	scoreTotNumText = Text_Create(4);
	if (!scoreTotNumText) goto f_scoreTotNumText;
	{
		int overall = 0;
		for (Tracker_Stat i = TRACKER_LVL1; i <= TRACKER_LVL18; i++) {
			overall += Tracker_Get(i);
		}
		Text_SetContent(scoreTotNumText, "%+i", overall);
	}

	nameText = Text_Create(EDITOR_LEVEL_NAME_MAX + 5);
	if (!nameText) goto f_nameText;
	Text_SetContent(nameText, name);

	if (params->isSequence) {
		nextLevel = params->level + 1;
		while (true) {
			char path[LEVEL_PATH_MAX];
			LevelIO_MakePath(nextLevel, params->levelInRomfs,
					path);
			if (FILE *f = fopen(path, "rb")) {
				fclose(f);
				break;
			}
			if (nextLevel >= SAVEDATA_NUM_LEVELS) {
				nextLevel = -1;
				break;
			}
			nextLevel++;
		}
	}

	Music_Start(MUSIC_RESULTS);

	level = params->level;
	levelInRomfs = params->levelInRomfs;
	isSequence = params->isSequence;
	strokes = params->strokes;

	textRevealCounter = 0;
	projPath = params->projPath;

	free(name);
	return true;

f_nameText:
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
	Scene_Switch(sceneError, &(Error_Params) { "Out of memory" });
	free(name);
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
	Text_Free(nameText);
	Tracer_Free(projPath);
	Terrain_Exit();
	// Music_Stop called contextually in nextScene
}

static void nextScene() {
	if (isSequence) {
		if (nextLevel >= 0) {	
			Music_Stop();
			Scene_Switch(sceneCourse,
					&(Course_Params) { nextLevel, levelInRomfs,
						isSequence });
		} else {
			Scene_Switch(sceneSummary,
					&(Summary_Params) { levelInRomfs });
		}
	} else {
		Music_Stop();
		Scene_Switch(sceneLevelSelector, &(LevelSelector_Params) { level });
	}
}

static void sceneUpdate(float _) {
	u32 kDown = hidKeysDown();

	textRevealCounter = clamp(textRevealCounter + 1, 0, TIMER_MAX);
	if (textRevealCounter == TIMER_REVEAL_PAR
			|| textRevealCounter == TIMER_REVEAL_STROKES
			|| textRevealCounter == TIMER_REVEAL_SCORE) {
		SoundEffect_Play(SFX_BOUNCE, true);
	}

	if (kDown & (KEY_A | KEY_B | KEY_X | KEY_Y)
			|| TouchInput_JustFinished()) {
		if (textRevealCounter < TIMER_MAX) {
			textRevealCounter = TIMER_MAX;
		} else {
			nextScene();
		}
	}
}

static u32 getColorForScore(int strokes, int par) {
	int score = strokes - par;
	if (strokes == 1) {
		return COLOR_WHITE;
	} else if (score <= -2) {
		return COLOR_YELLOW;
	} else if (score <= -1) {
		return COLOR_LBLUE;
	} else if (score <= 0) {
		return COLOR_LGREEN;
	} else {
		return COLOR_TAN;
	}
}

static void sceneDraw() {
	Tracer_UpdateGraphics(projPath);


	int terrainX, terrainY, terrainWidth, terrainHeight;
	#define D3D_DEPTHS { 0.8, 0, 0.6, 0.8 }
	#define D3D_XS { \
			200, \
			LEVEL_PREVIEW_X, \
			terrainX + ceilf(D3D_CORRECTION(2)), \
			terrainX \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, COLOR_LGRAY); \
	C2D_SceneBegin(D3D_TARGET); \
	\
	Text_Draw(completeText, D3D_Xi(0), COMPLETE_TEXT_Y, D3D_D(0), COLOR_DGREEN, \
			2, TEXT_CENTER); \
	\
	Terrain_Draw(D3D_Xi(1), LEVEL_PREVIEW_Y, D3D_D(1), LEVEL_PREVIEW_WIDTH, \
			LEVEL_PREVIEW_HEIGHT, &terrainX, &terrainY, &terrainWidth, \
			&terrainHeight); \
	Border_DrawLight(D3D_Xi(2), terrainY, D3D_D(2), \
			terrainWidth - 2*ceilf(D3D_CORRECTION(2)), terrainHeight); \
	Tracer_Draw(projPath, D3D_X(3), terrainY, D3D_D(3), terrainWidth, \
			terrainHeight);
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	Border_DrawDark(BOX_X, BOX_Y, 0, BOX_WIDTH, BOX_HEIGHT);
	C2D_DrawRectSolid(BOX_X, BOX_Y, 0, BOX_WIDTH, BOX_HEIGHT, COLOR_DGRAY);
	Text_Draw(nameText, 160, NAME_TEXT_Y, 0, COLOR_LGRAY, 1, TEXT_CENTER);
	if (textRevealCounter >= TIMER_REVEAL_PAR) {
		Text_Draw(parText, PAR_TEXT_X, PAR_TEXT_Y, 0, COLOR_LGRAY, 1,
				TEXT_LEFT);
		Text_Draw(parNumText, PAR_TEXT_X + NUM_TEXT_GAP, PAR_TEXT_Y, 0,
				COLOR_LGRAY, 1, TEXT_RIGHT);
	}
	if (textRevealCounter >= TIMER_REVEAL_STROKES) {
		Text_Draw(strokesText, STROKES_TEXT_X, STROKES_TEXT_Y, 0,
				COLOR_LGRAY, 1, TEXT_LEFT);
		Text_Draw(strokesNumText, STROKES_TEXT_X + NUM_TEXT_GAP,
				STROKES_TEXT_Y, 0, COLOR_LGRAY, 1, TEXT_RIGHT);
	}
	if (textRevealCounter >= TIMER_REVEAL_SCORE) {
		Text_Draw(scoreTotText, OVERALL_TEXT_X, OVERALL_TEXT_Y, 0,
				COLOR_DGRAY, 1, TEXT_LEFT);
		Text_Draw(scoreTotNumText, OVERALL_TEXT_X + NUM_TEXT_GAP,
				OVERALL_TEXT_Y, 0, COLOR_DGRAY, 1, TEXT_RIGHT);
		Text_Draw(scoreNameText, 160, SCORE_TEXT_Y, 0,
				getColorForScore(strokes, par), 1, TEXT_CENTER);
	}

	Animation_Draw(0.5);
}

Scene sceneResults = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

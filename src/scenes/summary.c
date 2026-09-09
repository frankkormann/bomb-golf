#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "summary.h"
#include "editor.h"
#include "error.h"
#include "title.h"
#include "components/text.h"
#include "components/border.h"
#include "../rendering/rendertarget.h"
#include "../rendering/color.h"
#include "../rendering/draw3d.h"
#include "../audio/music.h"
#include "../audio/soundeffect.h"
#include "../util/tracker.h"
#include "../util/macros.h"
#include "../util/touchinput.h"
#include "../levelio.h"

#define BORDER_MARGIN_HORIZ	10
#define BORDER_MARGIN_VERT	5
#define SCORES_WIDTH		230
#define SCORES_NAME_WIDTH	200

#define SCORES_TOP_X		85
#define SCORES_TOP_Y_START	30

#define SCORES_BOT_X		45
#define SCORES_BOT_Y_START	5
#define OVERALL_X		SCORES_BOT_X
#define OVERALL_Y \
	(SCORES_BOT_Y_START + TEXT_LINE_HEIGHT*8 + BORDER_MARGIN_VERT + 10)

#define SCORE_REVEAL_TIME	15
#define TIMER_MAX		(18*SCORE_REVEAL_TIME + 1)

static Text nameText[18], scoreText[18], bottomText, totalScoreText, killCountText;

static u32 bgColor, fgColor;
static int timer;
static bool inRomfs;

static bool sceneInit(void *sceneParams) {
	Summary_Params *params = (Summary_Params*)sceneParams;

	int i;
	for (i = 0; i < 18; i++) {
		nameText[i] = Text_Create(EDITOR_LEVEL_NAME_MAX + 1);
		if (!nameText[i]) goto f_nameText;

		char *s, path[LEVEL_PATH_MAX];
		LevelIO_MakePath(i, params->inRomfs, path);
		if (LevelIO_ReadName(path, &s)) {
			Text_SetContent(nameText[i], s);
			free(s);
		} else {
			Text_SetContent(nameText[i], "");
		}
	}

	int j;
	for (j = 0; j < 18; j++) {
		scoreText[j] = Text_Create(4);
		if (!scoreText[j]) goto f_scoreText;
		Text_SetContent(scoreText[j], "%+i", Tracker_Get(TRACKER_LVL1 + j));
	}

	bottomText = Text_Create(32);
	if (!bottomText) goto f_bottomText;
	Text_SetContent(bottomText, "Overall Score\nAnimals Killed");
	
	totalScoreText = Text_Create(4);
	if (!totalScoreText) goto f_totalScoreText;
	{
		int overall = 0;
		for (Tracker_Stat i = TRACKER_LVL1; i <= TRACKER_LVL18; i++) {
			overall += Tracker_Get(i);
		}
		Text_SetContent(totalScoreText, "%+i", overall);

		if (overall <= -6) {
			bgColor = COLOR_YELLOW;
			fgColor = COLOR_DGRAY;
		} else if (overall <= 0) {
			bgColor = COLOR_LGREEN;
			fgColor = COLOR_DGRAY;
		} else {
			bgColor = COLOR_DGRAY;
			fgColor = COLOR_LGRAY;
		}
	}

	killCountText = Text_Create(4);
	if (!killCountText) goto f_killCountText;
	Text_SetContent(killCountText, "%i", Tracker_Get(TRACKER_KILLS));

	timer = -1;  // So it's incremented to 0 on the first pass
	inRomfs = params->inRomfs;

	Music_Start(MUSIC_SUMMARY);

	return true;

f_killCountText:
	Text_Free(totalScoreText);
f_totalScoreText:
	Text_Free(bottomText);
f_bottomText:
f_scoreText:
	for (int k = 0; k < j; k++) Text_Free(scoreText[k]);
f_nameText:
	for (int k = 0; k < i; k++) Text_Free(nameText[k]);
	Scene_Switch(sceneError, &(Error_Params) { "Out of memory" });
	return false;
}

static void sceneExit() {
	for (int i = 0; i < 18; i++) {
		Text_Free(nameText[i]);
		Text_Free(scoreText[i]);
	}
	Text_Free(bottomText);
	Text_Free(totalScoreText);
	Text_Free(killCountText);
	Music_Stop();
}

static void sceneUpdate(float _) {
	u32 kDown = hidKeysDown();

	timer = clamp(timer + 1, 0, TIMER_MAX);
	if (timer % SCORE_REVEAL_TIME == 0 && timer / SCORE_REVEAL_TIME < 18) {
		SoundEffect_Play(SFX_UI_ADVANCE, false);
	}

	if (kDown & (KEY_A | KEY_B | KEY_X | KEY_Y)
			|| TouchInput_JustFinished()) {
		if (timer < TIMER_MAX) {
			timer = TIMER_MAX;
		} else {
			Scene_Switch(sceneTitle, &(Title_Params) SCENE_PARAMS_EMPTY);
		}
	}
}

static u32 getColorForScore(int score) {
	if (score <= -2) {
		return COLOR_YELLOW;
	} else if (score <= 0) {
		return COLOR_GREEN;
	} else {
		return COLOR_DBROWN;
	}
}

static void sceneDraw() {
	#define D3D_VALS { \
			{ SCORES_TOP_X, 0.6 }, \
			{ SCORES_TOP_X + SCORES_WIDTH, 0.6 }, \
			{ SCORES_TOP_X - BORDER_MARGIN_HORIZ, 0.8 } \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, bgColor); \
	C2D_SceneBegin(D3D_TARGET); \
	\
	C2D_DrawRectSolid(D3D_Xi(2), SCORES_TOP_Y_START - BORDER_MARGIN_VERT, -1, \
			SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ, 240, COLOR_LGRAY); \
	for (int i = 0; i < 10; i++) { \
		if (timer < SCORE_REVEAL_TIME*i) break; \
		u32 scoreColor = getColorForScore(Tracker_Get(TRACKER_LVL1 + i)); \
		Text_DrawBounded(nameText[i], D3D_Xi(0), \
				SCORES_TOP_Y_START + i*TEXT_LINE_HEIGHT, D3D_D(0), \
				SCORES_NAME_WIDTH, COLOR_DGREEN, 1, TEXT_LEFT); \
		Text_Draw(scoreText[i], D3D_Xi(1), \
				SCORES_TOP_Y_START + i*TEXT_LINE_HEIGHT, D3D_D(1), \
				scoreColor, 1, TEXT_RIGHT); \
	} \
	Border_DrawLight(D3D_Xi(2), SCORES_TOP_Y_START - BORDER_MARGIN_VERT, \
			D3D_D(2), SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ, 240);
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, bgColor);
	C2D_SceneBegin(bottom);

	C2D_DrawRectSolid(SCORES_BOT_X - BORDER_MARGIN_HORIZ, 0, 0,
			SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ,
			SCORES_BOT_Y_START + 8*TEXT_LINE_HEIGHT
				+ BORDER_MARGIN_VERT, COLOR_LGRAY);
	for (int i = 0; i < 8; i++) {
		if (timer < SCORE_REVEAL_TIME*(i + 10)) break;
		u32 scoreColor = getColorForScore(Tracker_Get(TRACKER_LVL1 + i+10));
		Text_DrawBounded(nameText[i+10], SCORES_BOT_X,
				SCORES_BOT_Y_START + i*TEXT_LINE_HEIGHT, 0,
				SCORES_NAME_WIDTH, COLOR_DGREEN, 1, TEXT_LEFT);
		Text_Draw(scoreText[i+10], SCORES_BOT_X + SCORES_WIDTH,
				SCORES_BOT_Y_START + i*TEXT_LINE_HEIGHT, 0,
				scoreColor, 1, TEXT_RIGHT);
	}
	Border_DrawLight(SCORES_BOT_X - BORDER_MARGIN_HORIZ, 0, 0,
			SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ,
			SCORES_BOT_Y_START + 8*TEXT_LINE_HEIGHT
				+ BORDER_MARGIN_VERT);

	if (timer >= SCORE_REVEAL_TIME*18) {
		Text_Draw(bottomText, OVERALL_X, OVERALL_Y, 0, fgColor, 1,
				TEXT_LEFT);
		Text_Draw(totalScoreText, OVERALL_X + SCORES_WIDTH, OVERALL_Y, 0,
				fgColor, 1, TEXT_RIGHT);
		Text_Draw(killCountText, OVERALL_X + SCORES_WIDTH,
				OVERALL_Y + TEXT_LINE_HEIGHT, 0, fgColor, 1,
				TEXT_RIGHT);
	}
}

Scene sceneSummary = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

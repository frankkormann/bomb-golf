#include <math.h>
#include <stdbool.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "summary.h"
#include "editor.h"
#include "error.h"
#include "credits.h"
#include "title.h"
#include "components/text.h"
#include "components/border.h"
#include "../rendering/rendertarget.h"
#include "../rendering/color.h"
#include "../rendering/draw3d.h"
#include "../rendering/animation.h"
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
#define SCORES_TOP_Y_START	25

#define SCORES_BOT_X		45
#define SCORES_BOT_Y_START	5
#define OVERALL_X		SCORES_BOT_X
#define OVERALL_Y \
	(SCORES_BOT_Y_START + TEXT_LINE_HEIGHT*8 + BORDER_MARGIN_VERT + 10)

#define SCORE_REVEAL_TIME	15

static Text nameText[18], scoreText[18], bottomText, totalScoreText, killCountText;
static int numScores;

static u32 bgColor, fgColor;
static int timer;
static bool inRomfs;

static bool sceneInit(void *sceneParams) {
	Summary_Params *params = (Summary_Params*)sceneParams;

	int i;
	numScores = 0;
	for (i = 0; i < 18; i++) {
		char *s, path[LEVEL_PATH_MAX];
		LevelIO_MakePath(i, params->inRomfs, path);
		if (LevelIO_ReadName(path, &s)) {
			nameText[i] = Text_Create(EDITOR_LEVEL_NAME_MAX + 1);
			if (!nameText[i]) goto f_nameScoreText;
			scoreText[i] = Text_Create(4);
			if (!scoreText[i]) goto f_nameScoreText;

			Text_SetContent(nameText[i], s);
			Text_SetContent(scoreText[i], "%+i",
					Tracker_Get(TRACKER_LVL1 + i));
			numScores++;
		} else {
			nameText[i] = NULL;
			scoreText[i] = NULL;
		}
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
			fgColor = COLOR_DGREEN;
		} else if (overall <= 0) {
			bgColor = COLOR_BLUE;
			fgColor = COLOR_LGRAY;
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

	return true;

f_killCountText:
	Text_Free(totalScoreText);
f_totalScoreText:
	Text_Free(bottomText);
f_bottomText:
f_nameScoreText:
	for (int j = 0; j < i; j++) {
		if (nameText[j]) Text_Free(nameText[j]);
		if (scoreText[j]) Text_Free(scoreText[j]);
	}
	Scene_Switch(sceneError, &(Error_Params) { "Out of memory" });
	return false;
}

static void sceneExit() {
	for (int i = 0; i < 18; i++) {
		if (nameText[i]) Text_Free(nameText[i]);
		if (scoreText[i]) Text_Free(scoreText[i]);
	}
	Text_Free(bottomText);
	Text_Free(totalScoreText);
	Text_Free(killCountText);
	Music_Stop();
}

static void sceneUpdate(float _) {
	u32 kDown = hidKeysDown();

	timer = clamp(timer + 1, 0, (numScores + 1) * SCORE_REVEAL_TIME);
	if (timer % SCORE_REVEAL_TIME == 0
			&& timer / SCORE_REVEAL_TIME < numScores) {
		SoundEffect_Play(SFX_BOUNCE, true);
	}

	if (kDown & (KEY_A | KEY_B | KEY_X | KEY_Y)
			|| TouchInput_JustFinished()) {
		if (timer < (numScores + 1) * SCORE_REVEAL_TIME) {
			timer = (numScores + 1) * SCORE_REVEAL_TIME;
		} else if (inRomfs) {
			Scene_Switch(sceneCredits,
					&(Credits_Params) SCENE_PARAMS_EMPTY);
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
	int i, numDrawn;

	#define D3D_VALS { \
			{ SCORES_TOP_X, 0.8 }, \
			{ SCORES_TOP_X + SCORES_WIDTH, 0.8 }, \
			{ SCORES_TOP_X - BORDER_MARGIN_HORIZ, 0.6 } \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, bgColor); \
	C2D_SceneBegin(D3D_TARGET); \
	\
	i = 0; \
	for (numDrawn = 0; numDrawn < 10 && i < 18; i++) { \
		if (!nameText[i] || !scoreText[i]) continue; \
		u32 scoreColor = getColorForScore(Tracker_Get(TRACKER_LVL1 + i)); \
		if (timer >= numDrawn * SCORE_REVEAL_TIME) { \
			int y = SCORES_TOP_Y_START + numDrawn*TEXT_LINE_HEIGHT; \
			/* Make sure the background looks good underneath the \
			   text, sometimes there are issues due to transparency */ \
			C2D_DrawRectSolid(D3D_Xi(2), y, D3D_D(2), \
					SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ, \
					TEXT_LINE_HEIGHT, COLOR_LGRAY); \
			Text_DrawBounded(nameText[i], D3D_Xi(0), y, D3D_D(0), \
					SCORES_NAME_WIDTH, COLOR_DGREEN, 1, \
					TEXT_LEFT); \
			Text_Draw(scoreText[i], D3D_Xi(1), y, D3D_D(1), scoreColor, \
					1, TEXT_RIGHT); \
		} \
		numDrawn++; \
	} \
	int boxHeight = (numDrawn * TEXT_LINE_HEIGHT) + 2*BORDER_MARGIN_VERT; \
	if (numDrawn < numScores) boxHeight += 20; \
	C2D_DrawRectSolid(D3D_Xi(2), SCORES_TOP_Y_START - BORDER_MARGIN_VERT, \
			D3D_D(2), SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ, \
			boxHeight, COLOR_LGRAY); \
	Border_DrawLight(D3D_Xi(2), SCORES_TOP_Y_START - BORDER_MARGIN_VERT, \
			D3D_D(2), SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ, boxHeight);
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, bgColor);
	C2D_SceneBegin(bottom);

	int topTime = numDrawn * SCORE_REVEAL_TIME;
	for (numDrawn = 0; numDrawn < 8 && i < 18; i++) {
		if (!nameText[i] || !scoreText[i]) continue;
		u32 scoreColor = getColorForScore(Tracker_Get(TRACKER_LVL1 + i));
		if (timer >= numDrawn * SCORE_REVEAL_TIME + topTime) {
			int y = SCORES_BOT_Y_START + numDrawn*TEXT_LINE_HEIGHT;
			// Make sure the background looks good underneath the
			// text, sometimes there are issues due to transparency
			C2D_DrawRectSolid(SCORES_BOT_X - BORDER_MARGIN_HORIZ, y, 0,
					SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ,
					TEXT_LINE_HEIGHT, COLOR_LGRAY);
			Text_DrawBounded(nameText[i], SCORES_BOT_X, y, 0.5,
					SCORES_NAME_WIDTH, COLOR_DGREEN, 1,
					TEXT_LEFT);
			Text_Draw(scoreText[i], SCORES_BOT_X + SCORES_WIDTH, y,
					0.5, scoreColor, 1, TEXT_RIGHT);
		}
		numDrawn++;
	}
	if (numDrawn > 0) {
	int boxHeight = (numDrawn * TEXT_LINE_HEIGHT) + 2*BORDER_MARGIN_VERT;
	C2D_DrawRectSolid(SCORES_BOT_X - BORDER_MARGIN_HORIZ,
			SCORES_BOT_Y_START - BORDER_MARGIN_VERT, 0,
			SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ, boxHeight,
			COLOR_LGRAY);
	Border_DrawLight(SCORES_BOT_X - BORDER_MARGIN_HORIZ,
			SCORES_BOT_Y_START - BORDER_MARGIN_VERT, 0,
			SCORES_WIDTH + 2*BORDER_MARGIN_HORIZ, boxHeight);
	}

	if (timer >= SCORE_REVEAL_TIME * (numScores + 1)) {
		Text_Draw(bottomText, OVERALL_X, OVERALL_Y, 0, fgColor, 1,
				TEXT_LEFT);
		Text_Draw(totalScoreText, OVERALL_X + SCORES_WIDTH, OVERALL_Y, 0,
				fgColor, 1, TEXT_RIGHT);
		Text_Draw(killCountText, OVERALL_X + SCORES_WIDTH,
				OVERALL_Y + TEXT_LINE_HEIGHT, 0, fgColor, 1,
				TEXT_RIGHT);
	}

	Animation_Draw(0.5);
}

Scene sceneSummary = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

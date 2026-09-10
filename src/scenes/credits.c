#include <stdbool.h>
#include <string.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "credits.h"
#include "title.h"
#include "error.h"
#include "components/text.h"
#include "../rendering/color.h"
#include "../rendering/rendertarget.h"
#include "../rendering/draw3d.h"
#include "../rendering/animation.h"
#include "../util/macros.h"
#include "../util/touchinput.h"

#define SCROLL_SPEED 0.5
// Stop when "Thanks for Playing!" is the only line showing
#define SCROLL_MAX 480 + (13 * TEXT_LINE_HEIGHT)

#define CREDITS_TEXT_HEADERS \
	"Design, Program, Art\n" \
	"\n" \
	"\n" \
	"Music\n" \
	"\n" \
	"\n" \
	"Special Thanks\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n"
#define CREDITS_TEXT_CONTENT \
	"\n" \
	"Frank Kormann\n" \
	"\n" \
	"\n" \
	"Kevin MacLeod\n" \
	"\n" \
	"\n" \
	"Contributors to devkitPro, libctru,\n" \
	"libopusfile, and citro2D\n" \
	"\n" \
	"\n" \
	"No birds were harmed in the making\n" \
	"of this game\n" \
	"\n" \
	"\n" \
	"Thanks for Playing!\n"

static Text headersText, contentText;
static float scroll;

static bool sceneInit() {
	headersText = Text_Create(strlen(CREDITS_TEXT_HEADERS) + 1);
	if (!headersText) goto f_headersText;
	Text_SetContent(headersText, CREDITS_TEXT_HEADERS);

	contentText = Text_Create(strlen(CREDITS_TEXT_CONTENT) + 1);
	if (!contentText) goto f_contentText;
	Text_SetContent(contentText, CREDITS_TEXT_CONTENT);

	scroll = 0;
	return true;

f_contentText:
	Text_Free(headersText);
f_headersText:
	Scene_Switch(sceneError, &(Error_Params) { "Out of memory" });
	return false;	
}

static void sceneExit() {
	Text_Free(headersText);
	Text_Free(contentText);
}

static void sceneUpdate(float speed) {
	//TODO Allow tapping the screen to blow up parts of the text
	u32 kDown = hidKeysDown();

	scroll = clamp(scroll + speed * SCROLL_SPEED, 0, SCROLL_MAX);

	if ((kDown & (KEY_A | KEY_B | KEY_X | KEY_Y)
				|| TouchInput_JustFinished())
			&& scroll >= SCROLL_MAX) {
		Scene_Switch(sceneTitle, &(Title_Params) SCENE_PARAMS_EMPTY);
	}
}

static void sceneDraw() {
	float textY = 480 - scroll;


	#define D3D_VALS { \
			{ 200, 0.8 } \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, COLOR_LGRAY); \
	C2D_SceneBegin(D3D_TARGET); \
	Text_Draw(headersText, D3D_Xi(0), textY, D3D_D(0), COLOR_DGRAY, 1, \
			TEXT_CENTER); \
	Text_Draw(contentText, D3D_Xi(0), textY, D3D_D(0), COLOR_DGREEN, 1, \
			TEXT_CENTER);
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);
	Text_Draw(headersText, 160, textY - 240, 0, COLOR_DGRAY, 1, TEXT_CENTER);
	Text_Draw(contentText, 160, textY - 240, 0, COLOR_DGREEN, 1, TEXT_CENTER);

	//TODO Maybe some art here also?

	Animation_Draw(0.5);
}

Scene sceneCredits = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

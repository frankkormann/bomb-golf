#include <string.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "error.h"
#include "title.h"
#include "../rendering/rendertarget.h"
#include "../rendering/color.h"

#define GLYPH_KEY_A ""

static C2D_Text errText, infoText;
static C2D_TextBuf textBuf;

static bool sceneInit(void *sceneParams) {
	Error_Params *params = (Error_Params*)sceneParams;
	// Assume 1 character -> at most 1 glyph
	textBuf = C2D_TextBufNew(
			strlen(params->msg) + 1  // for errText
			+ 16                          // for infoText
		);
	if (!textBuf) goto f_textBuf;

	C2D_TextParse(&errText, textBuf, params->msg);
	C2D_TextParse(&infoText, textBuf, GLYPH_KEY_A ": Go to title");
	C2D_TextOptimize(&errText);
	C2D_TextOptimize(&infoText);

	return true;

f_textBuf:
	return false;
}

static void sceneUpdate(float _) {
	u32 kDown = hidKeysDown();
	if (kDown & KEY_A) {
		Scene_Switch(sceneTitle, &(Title_Params) SCENE_PARAMS_EMPTY);
	}
}

static void sceneDraw() {
	C3D_RenderTarget *top = RenderTarget_Left();
	C2D_TargetClear(top, COLOR_WHITE);
	C2D_SceneBegin(top);

	C2D_DrawText(&errText, 0, 50, 50, 0, 0.5, 0.5);
	C2D_DrawText(&infoText, 0, 50, 180, 0, 0.5, 0.5);

	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_WHITE);
	C2D_SceneBegin(bottom);
}

static void sceneExit() {
	C2D_TextBufDelete(textBuf);
}

Scene sceneError = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

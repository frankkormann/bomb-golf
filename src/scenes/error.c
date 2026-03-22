#include <string.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "error.h"
#include "../rendering/rendertarget.h"
#include "../rendering/colors.h"
#include "../util/macros.h"
#include "../util/keychars.h"

static C2D_Text errText, infoText;
static C2D_TextBuf textBuf;

Scene_Params Error_MakeParams(const char *msg) {
	Error_Params params;
	params.msg = malloc(strlen(msg) + sizeof('\0'));
	strcpy(params.msg, msg);
	return (Scene_Params) { .error = params };
}

static bool sceneInit(Scene_Params params) {
	// Assume 1 character -> at most 1 glyph
	textBuf = C2D_TextBufNew(
			strlen(params.error.msg) + 1  // for errText
			+ 16                          // for infoText
		);
	if (!textBuf) goto f_textBuf;

	C2D_TextParse(&errText, textBuf, params.error.msg);
	C2D_TextParse(&infoText, textBuf, KEYCHAR_A ": Go to title");
	C2D_TextOptimize(&errText);
	C2D_TextOptimize(&infoText);

	free(params.error.msg);
	return true;

f_textBuf:
	free(params.error.msg);
	return false;
}

static void sceneUpdate() {
	u32 kDown = hidKeysDown();
	if (kDown & KEY_A) {
		Scene_SetNext(sceneTitle, Title_MakeParams());
	}
}

static void sceneDraw() {
	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_WHITE);
	C2D_SceneBegin(top);

	C2D_DrawText(&errText, 0, 50, 50, 0, 0.5, 0.5);
	C2D_DrawText(&infoText, 0, 50, 180, 0, 0.5, 0.5);

	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
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

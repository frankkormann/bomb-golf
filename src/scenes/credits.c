#include <stdbool.h>
#include <malloc.h>
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
#include "../rendering/spritesheet.h"
#include "../rendering/animations/explosion.h"
#include "../audio/soundeffect.h"
//#include "../audio/music.h" //TODO
#include "../util/macros.h"
#include "../util/touchinput.h"

#define SCROLL_SPEED 0.4
// Compensates for buffer at the beginning of each texture
#define SCROLL_MIN 10
// Stop when "Thanks For Playing!" is the only line showing
#define SCROLL_MAX (480 + (14 * TEXT_LINE_HEIGHT) + SCROLL_MIN)

#define CREDITS_TEXT_HEADERS \
	"Design, Program, Art\n" \
	"\n" \
	"\n" \
	"Music\n" \
	"\n" \
	"\n" \
	"\n" \
	"Special Thanks\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n" \
	"No birds were harmed in the making\n" \
	"of this game\n" \
	"\n" \
	"\n" \
	"\n"
#define CREDITS_TEXT_CONTENT \
	"\n" \
	"Frank Kormann\n" \
	"\n" \
	"\n" \
	"Kevin MacLeod\n" \
	"(incompetech.com)\n" \
	"\n" \
	"\n" \
	"All contributors to devkitPro, libctru,\n" \
	"libopusfile, and citro2D\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n" \
	"\n" \
	"Thanks For Playing!\n"

#define EXPLOSION_RADIUS 20

static Text headersText, contentText;
static float scroll;
static C3D_Tex layer1Tex, layer2Tex;
static Tex3DS_SubTexture subtex;
static C3D_RenderTarget *layer1Target, *layer2Target;
static float explodeX, explodeY;
static bool shouldSetupTex;

static bool sceneInit() {
	// Tex dimensions must be a power of 2
	int texWidth = 512;
	int texHeight = __builtin_stdc_bit_ceil((unsigned int)SCROLL_MAX);
	if (!C3D_TexInitVRAM(&layer1Tex, texWidth, texHeight, GPU_RGBA8))
		goto f_layer1Tex;
	if (!C3D_TexInitVRAM(&layer2Tex, texWidth, texHeight, GPU_RGBA8))
		goto f_layer2Tex;

	subtex = (Tex3DS_SubTexture) {
			.width	= 320,
			.height = SCROLL_MAX,
			.left	= 0,
			.right	= (float)320/ texWidth,
			.top	= 1,
			.bottom	= 1 - ((float)SCROLL_MAX / texHeight)
		};

	layer1Target = C3D_RenderTargetCreateFromTex(&layer1Tex, GPU_TEXFACE_2D, 0,
			-1);
	if (!layer1Target) goto f_layer1Target;
	layer2Target = C3D_RenderTargetCreateFromTex(&layer2Tex, GPU_TEXFACE_2D, 0,
			-1);
	if (!layer2Target) goto f_layer2Target;

	headersText = Text_Create(strlen(CREDITS_TEXT_HEADERS) + 1);
	if (!headersText) goto f_headersText;
	Text_SetContent(headersText, CREDITS_TEXT_HEADERS);

	contentText = Text_Create(strlen(CREDITS_TEXT_CONTENT) + 1);
	if (!contentText) goto f_contentText;
	Text_SetContent(contentText, CREDITS_TEXT_CONTENT);

	scroll = SCROLL_MIN;
	shouldSetupTex = true;
	explodeX = explodeY = -1;
	return true;

f_contentText:
	Text_Free(headersText);
f_headersText:
	C3D_RenderTargetDelete(layer2Target);
f_layer2Target:
	C3D_RenderTargetDelete(layer1Target);
f_layer1Target:
	C3D_TexDelete(&layer2Tex);
f_layer2Tex:
	C3D_TexDelete(&layer1Tex);
f_layer1Tex:
	Scene_Switch(sceneError, &(Error_Params) { "Out of memory" });
	return false;	
}

static void sceneExit() {
	Text_Free(headersText);
	Text_Free(contentText);
	C3D_TexDelete(&layer1Tex);
	C3D_TexDelete(&layer2Tex);
	C3D_RenderTargetDelete(layer1Target);
	C3D_RenderTargetDelete(layer2Target);
}

static void sceneUpdate(float speed) {
	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	if (kHeld & KEY_UP) scroll += 5 * speed;
	if (kHeld & KEY_DOWN) scroll -= 5 * speed;
	scroll = clamp(scroll + speed * SCROLL_SPEED, SCROLL_MIN, SCROLL_MAX);

	if ((kDown & (KEY_A | KEY_B | KEY_X | KEY_Y))
			&& scroll >= SCROLL_MAX) {
		Scene_Switch(sceneTitle, &(Title_Params) SCENE_PARAMS_EMPTY);
	}

	if (TouchInput_JustStarted()) {
		TouchInput_Swipe touch = TouchInput_GetSwipe();
		explodeX = touch.start.px;
		explodeY = touch.start.py - 240 + scroll;
		Animation_Start(animationExplosion,
				Explosion_MakeParams(explodeX, explodeY,
					EXPLOSION_RADIUS), NULL);
		SoundEffect_Play(SFX_EXPLOSION, true);
	}
}

static void fillCircle(int x, int y, float depth, int radius, u32 color) {
	for (int cx = x - radius; cx < x + radius; cx++) {
		for (int cy = y - radius; cy < y + radius; cy++) {
			int rx = x - cx;
			int ry = y - cy;
			if (rx*rx + ry*ry < radius*radius) {
				// There is a forbidden zone at the top of the
				// texture where the 3DS hangs if you try to
				// draw to it
				if (cy < SCROLL_MIN) continue;
				// Don't draw above bottom screen
				if (cy < scroll - 240) continue;
				C2D_DrawRectSolid(cx, cy, depth, 1, 1, color);
			}
		}
	}
}

static void updateLayers() {
	if (shouldSetupTex) {
		shouldSetupTex = false;

		// For some reason, drawing at y=0 here doesn't display
		// So we have to put a buffer at the beginning of the texture
		C2D_TargetClear(layer1Target, COLOR_TRANSPARENT);
		C2D_SceneBegin(layer1Target);
		Text_Draw(headersText, 160, SCROLL_MIN, 0, COLOR_DGRAY, 1,
				TEXT_CENTER);
		SpriteSheet_Draw(SPRITE_CREDITS, 0, SCROLL_MAX - 420, 0, 0, false,
				false);

		C2D_TargetClear(layer2Target, COLOR_TRANSPARENT);
		C2D_SceneBegin(layer2Target);
		Text_Draw(contentText, 160, SCROLL_MIN, 0, COLOR_DGREEN, 1,
				TEXT_CENTER);
	}
	if (explodeX >= 0 && explodeY >= 0) {
		C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_MIN, GPU_SRC_ALPHA,
				GPU_ONE_MINUS_SRC_ALPHA,
				GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);

		C2D_SceneBegin(layer1Target);
		fillCircle(explodeX, explodeY, 0, EXPLOSION_RADIUS,
				COLOR_GREEN);
		C2D_SceneBegin(layer2Target);
		fillCircle(explodeX, explodeY, 0, EXPLOSION_RADIUS,
				COLOR_GREEN);

		C2D_Flush();
		C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA,
				GPU_ONE_MINUS_SRC_ALPHA,
				GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);

		explodeX = explodeY = -1;
	}
}

static void sceneDraw() {
	updateLayers();
	C2D_Image layer1 = { &layer1Tex, &subtex };
	C2D_Image layer2 = { &layer2Tex, &subtex };


	#define D3D_VALS { \
			{ 40, 0.6 }, \
			{ 40, 0.8 } \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, COLOR_LGRAY); \
	C2D_SceneBegin(D3D_TARGET); \
	C2D_ViewTranslate(0, 480 - scroll); \
	\
	C2D_DrawImageAt(layer1, D3D_Xi(0), 0, D3D_D(0), NULL, 1, 1); \
	C2D_DrawImageAt(layer2, D3D_Xi(1), 0, D3D_D(1), NULL, 1, 1); \
	\
	C2D_ViewReset();
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);
	C2D_ViewTranslate(0, 240 - scroll);

	C2D_DrawImageAt(layer1, 0, 0, 0, NULL, 1, 1);
	C2D_DrawImageAt(layer2, 0, 0, 0, NULL, 1, 1);

	Animation_Draw(0.5);
	C2D_ViewReset();
}

Scene sceneCredits = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

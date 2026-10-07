#include <stdbool.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "arsenal.h"
#include "course.h"
#include "title.h"
#include "components/text.h"
#include "components/button.h"
#include "../rendering/rendertarget.h"
#include "../rendering/color.h"
#include "../rendering/draw3d.h"
#include "../projectiles/bomb.h"
#include "../projectiles/missile.h"
#include "../projectiles/drill.h"
#include "../util/dispatcher.h"

#define INSTRUCTIONS_TEXT_Y	100

#define BUTTON_X		58
#define BUTTON_Y_START		70
#define BUTTON_GAP		35

static Button bombButton, missileButton, drillButton;
static Text   bombText,   missileText,   drillText, instructionsText;
static Dispatcher touchDispatcher;

static void selectProjectile(void *projectile) {
	Scene_Switch(sceneCourse, &(Course_Params) { 0, true, true,
			projectile });
}

static bool sceneInit() {
	bombButton = Button_Create(BUTTON_X, BUTTON_Y_START, SPRITE_LONG_BUTTON,
			-1, projectileBomb, selectProjectile);
	if (!bombButton) goto f_bombButton;

	missileButton = Button_Create(BUTTON_X, BUTTON_Y_START + BUTTON_GAP,
			SPRITE_LONG_BUTTON, -1, projectileMissile, selectProjectile);
	if (!missileButton) goto f_missileButton;

	drillButton = Button_Create(BUTTON_X, BUTTON_Y_START + 2*BUTTON_GAP,
			SPRITE_LONG_BUTTON, -1, projectileDrill, selectProjectile);
	if (!drillButton) goto f_drillButton;

	instructionsText = Text_Create(19);
	if (!instructionsText) goto f_instructionsText;
	Text_SetContent(instructionsText, "Choose Your Weapon");

	bombText = Text_Create(5);
	if (!bombText) goto f_bombText;
	Text_SetContent(bombText, "Bomb");

	missileText = Text_Create(8);
	if (!missileText ) goto f_missileText ;
	Text_SetContent(missileText , "Missile");

	drillText = Text_Create(6);
	if (!drillText ) goto f_drillText ;
	Text_SetContent(drillText , "Drill");

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	Button_RegisterForTouchEvents(bombButton, touchDispatcher, 0);
	Button_RegisterForTouchEvents(missileButton, touchDispatcher, 0);
	Button_RegisterForTouchEvents(drillButton, touchDispatcher, 0);

	return true;

f_touchDispatcher:
	Text_Free(drillText);
f_drillText:
	Text_Free(missileText);
f_missileText:
	Text_Free(bombText);
f_bombText:
	Text_Free(instructionsText);
f_instructionsText:
	Button_Free(drillButton);
f_drillButton:
	Button_Free(missileButton);
f_missileButton:
	Button_Free(bombButton);
f_bombButton:
	return false;
}

static void sceneExit() {
	Dispatcher_Free(touchDispatcher);
	Text_Free(missileText);
	Text_Free(bombText);
	Text_Free(drillText);
	Text_Free(instructionsText);
	Button_Free(missileButton);
	Button_Free(bombButton);
	Button_Free(drillButton);
}

static void sceneUpdate(float _) {
	u32 kDown = hidKeysDown();
	if (kDown & KEY_B) {
		Scene_Switch(sceneTitle, &(Title_Params) SCENE_PARAMS_EMPTY);
	}

	Dispatcher_DispatchEvent(touchDispatcher);
}

static void sceneDraw() {
	#define D3D_VALS { \
			{ 200, 0.8 } \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, COLOR_LGRAY); \
	C2D_SceneBegin(D3D_TARGET); \
	\
	Text_Draw(instructionsText, D3D_Xi(0), INSTRUCTIONS_TEXT_Y, 0, \
			COLOR_DGREEN, 2, TEXT_CENTER);
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */


	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);

	Button_Draw(bombButton, 0);
	Text_Draw(bombText, BUTTON_X + 10, BUTTON_Y_START + 5, 1, COLOR_LGRAY, 1,
			TEXT_LEFT);
	Button_Draw(missileButton, 0);
	Text_Draw(missileText, BUTTON_X + 10, BUTTON_Y_START + BUTTON_GAP + 5, 1,
			COLOR_LGRAY, 1, TEXT_LEFT);
	Button_Draw(drillButton, 0);
	Text_Draw(drillText, BUTTON_X + 10, BUTTON_Y_START + 2*BUTTON_GAP + 5, 1,
			COLOR_LGRAY, 1, TEXT_LEFT);
}

Scene sceneArsenal = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};
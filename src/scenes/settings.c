#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "settings.h"
#include "title.h"
#include "components/button.h"
#include "components/text.h"
#include "components/toggle.h"
#include "components/popup.h"
#include "../rendering/rendertarget.h"
#include "../rendering/color.h"
#include "../rendering/draw3d.h"
#include "../file/saveio.h"
#include "../util/dispatcher.h"
#include "../util/macros.h"
#include "../util/touchinput.h"

//TODO Consider touchscreen smoothing/sensitivity option

#define INFO_TEXT_X	30
#define INFO_TEXT_Y	20

#define DESC_X		30
#define BUTTON_X	190
#define OPTION_Y_START	20
#define OPTION_GAP	40
#define SAVE_BUTTON_X	110
#define SAVE_BUTTON_Y	(OPTION_Y_START + 3*OPTION_GAP + 10)
#define EXIT_BUTTON_X	-2
#define EXIT_BUTTON_Y	(240 - 30 + 2)

static Toggle mirrorToggle, explodeToggle;
static Button speedUpButton, speedDownButton, saveButton, exitButton;
static Text   speedUpText,   speedDownText,   saveText,   exitText,
		speedText, optionsText, infoText;
static Dispatcher touchDispatcher;

static float speed;
static bool isOptionSelected;

// Declarations needed for Buttons, Dispatcher
static void changeSpeed(void* multBits);
static void save(), gotoTitle();
static bool handleTouchInput();

static bool sceneInit() {
	bool isMirrored;
	SaveIO_ExplosionControls controls;
	if (!SaveIO_ReadSettings(&isMirrored, &controls, &speed)) {
		isMirrored = false;
		controls = CONTROLS_HOLD;
		speed = 1;
	}

	mirrorToggle = Toggle_Create(BUTTON_X, OPTION_Y_START, "Yes", "No",
			true, false, isMirrored);
	if (!mirrorToggle) goto f_mirrorToggle;

	speedUpButton = Button_Create(BUTTON_X + 80, OPTION_Y_START + OPTION_GAP,
			SPRITE_THIN_BUTTON, -1, (void*)0x3fc00000 /* 3/2 */,
			changeSpeed);
	if (!speedUpButton) goto f_speedUpButton;

	speedDownButton = Button_Create(BUTTON_X, OPTION_Y_START + OPTION_GAP,
			SPRITE_THIN_BUTTON, -1, (void*)0x3f2aaaab /* 2/3 */,
			changeSpeed);
	if (!speedDownButton) goto f_speedDownButton;

	explodeToggle = Toggle_Create(BUTTON_X, OPTION_Y_START + 2*OPTION_GAP,
			"Hold", "Taps", CONTROLS_HOLD, CONTROLS_TAPS,
			controls == CONTROLS_HOLD);
	if (!explodeToggle) goto f_explodeToggle;

	saveButton = Button_Create(SAVE_BUTTON_X, SAVE_BUTTON_Y,
			SPRITE_MEDIUM_BUTTON, -1, NULL, save);
	if (!saveButton) goto f_saveButton;

	exitButton = Button_Create(EXIT_BUTTON_X, EXIT_BUTTON_Y,
			SPRITE_SMALL_BUTTON, -1, NULL, gotoTitle);
	if (!exitButton) goto f_exitButton;

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	Toggle_RegisterForTouchEvents(mirrorToggle, touchDispatcher, 1);
	Toggle_RegisterForTouchEvents(explodeToggle, touchDispatcher, 1);
	Button_RegisterForTouchEvents(speedUpButton, touchDispatcher, 1);
	Button_RegisterForTouchEvents(speedDownButton, touchDispatcher, 1);
	Button_RegisterForTouchEvents(saveButton, touchDispatcher, 1);
	Button_RegisterForTouchEvents(exitButton, touchDispatcher, 1);
	Dispatcher_AddHandler(touchDispatcher,
			(Dispatcher_Handler) { 0, NULL, handleTouchInput });

	optionsText = Text_Create(64);
	if (!optionsText) goto f_optionsText;
	Text_SetContent(optionsText, "Mirror bottom screen\n\n"
			"Game speed\n\n"
			"Exploding controls");

	infoText = Text_Create(256);
	if (!infoText) goto f_infoText;
	Text_SetContent(infoText, "Tap an option to show help");

	speedUpText = Text_Create(2);
	if (!speedUpText) goto f_speedUpText;
	Text_SetContent(speedUpText, "+");

	speedDownText = Text_Create(2);
	if (!speedDownText) goto f_speedDownText;
	Text_SetContent(speedDownText, "-");

	speedText = Text_Create(8);
	if (!speedText) goto f_speedText;
	changeSpeed((void*)0x3f800000 /* 1.0 */);  // Set speedText's content

	saveText = Text_Create(5);
	if (!saveText) goto f_saveText;
	Text_SetContent(saveText, "Save");

	exitText = Text_Create(5);
	if (!exitText) goto f_exitText;
	Text_SetContent(exitText, "Back");

	isOptionSelected = false;

	return true;

f_exitText:
	Text_Free(saveText);
f_saveText:
	Text_Free(speedText);
f_speedText:
	Text_Free(speedDownText);
f_speedDownText:
	Text_Free(speedUpText);
f_speedUpText:
	Text_Free(infoText);
f_infoText:
	Text_Free(optionsText);
f_optionsText:
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	Button_Free(saveButton);
f_saveButton:
	Button_Free(exitButton);
f_exitButton:
	Toggle_Free(explodeToggle);
f_explodeToggle:
	Button_Free(speedDownButton);
f_speedDownButton:
	Button_Free(speedUpButton);
f_speedUpButton:
	Toggle_Free(mirrorToggle);
f_mirrorToggle:
	return false;
}

static void sceneExit() {
	Toggle_Free(mirrorToggle);
	Toggle_Free(explodeToggle);
	Button_Free(speedUpButton);
	Button_Free(speedDownButton);
	Button_Free(saveButton);
	Button_Free(exitButton);
	Text_Free(optionsText);
	Text_Free(infoText);
	Text_Free(speedUpText);
	Text_Free(speedDownText);
	Text_Free(speedText);
	Text_Free(saveText);
	Text_Free(exitText);
	Dispatcher_Free(touchDispatcher);
}

static void changeSpeed(void* multBits) {
	float mult = *((float*)&multBits);
	speed *= mult;
	speed = clamp(speed, (float)2/3, (float)3/2);
	Text_SetContent(speedText, "x%.2f", speed);
}

static void save() {
	if (SaveIO_WriteSettings(Toggle_GetValue(mirrorToggle),
			Toggle_GetValue(explodeToggle), speed)) {
		Popup_Init("Success!", POPUP_ONE_BUTTON,
				(Popup_Button[]) { { "Ok", -1, NULL, Popup_Exit } });
	} else {
		Popup_Init("Failed to save", POPUP_ONE_BUTTON,
				(Popup_Button[]) { { "Ok", -1, NULL, Popup_Exit } });
	}
}

static void gotoTitle() {
	Scene_Switch(sceneTitle, &(Title_Params) SCENE_PARAMS_EMPTY);
}

static bool pointInBox(touchPosition pos, int x, int y, int width, int height) {
	return pos.px >= x && pos.px < x + width && pos.py >= y
			&& pos.py < y + height;
}

static bool handleTouchInput() {
	if (!TouchInput_InProgress() && !TouchInput_JustFinished()) {
		return false;
	}

	TouchInput_Swipe touch = TouchInput_GetSwipe();
	if (!pointInBox(touch.start, DESC_X, OPTION_Y_START, BUTTON_X - DESC_X,
			3*OPTION_GAP)) {
		return false;
	}

	if (TouchInput_JustFinished()) {
		if (pointInBox(touch.end, DESC_X, OPTION_Y_START,
				BUTTON_X - DESC_X, 30)) {
			Text_SetContent(infoText, "Whether to reflect each level"
					" horizontally. This may\nmake it easier"
					" depending on how your hand blocks the"
					"\ntouchscreen while holding the stylus.");
			isOptionSelected = true;
			return true;
		}

		if (pointInBox(touch.end, DESC_X, OPTION_Y_START + OPTION_GAP,
				BUTTON_X - DESC_X, 30)) {
			Text_SetContent(infoText, "A multiplier for the base game"
					" speed. This option is\nintended to make"
					" it accessible for those with slower"
					"\nreaction times, not as a replacement"
					" for skill. Setting\nthis will NOT prevent"
					" achievements or high scores.");
			isOptionSelected = true;
			return true;
		}

		if (pointInBox(touch.end, DESC_X, OPTION_Y_START + 2*OPTION_GAP,
				BUTTON_X - DESC_X, 30)) {
			Text_SetContent(infoText, "How to explode the ball in"
					" mid-air.\n\n"
					"  Hold: Tap the touchscreen once and hold"
					" to aim,\n  then release to explode.\n\n"
					"  Taps: Tap the touchscreen once to enter"
					" aiming\n  mode. Tap again to explode.");
			isOptionSelected = true;
			return true;
		}
	}

	return true;

	Text_SetContent(infoText, "%i\n%i", touch.end.px, touch.end.py);
	isOptionSelected = true;
	return true;
}

static void sceneUpdate(float _) {
	u32 kDown = hidKeysDown();
	if (kDown & KEY_B) gotoTitle();
	Dispatcher_DispatchEvent(touchDispatcher);
}

static void sceneDraw() {
	#define D3D_VALS { \
			{ INFO_TEXT_X, 0.6 }, \
			{ 200, 0.6 } \
		}
	#define D3D_CODE \
	C2D_TargetClear(D3D_TARGET, COLOR_LGRAY); \
	C2D_SceneBegin(D3D_TARGET); \
	\
	if (isOptionSelected) { \
		Text_Draw(infoText, D3D_Xi(0), INFO_TEXT_Y, D3D_D(0), COLOR_DGRAY, \
				1, TEXT_LEFT); \
	} else { \
		Text_Draw(infoText, D3D_Xi(1), 60, D3D_D(1), COLOR_DGRAY, 1, \
				TEXT_CENTER); \
	}
	#include "../rendering/draw3d_gen.h"
	/* Everything gets #undef'd by draw3d */

	C3D_RenderTarget *bottom = RenderTarget_Bottom();
	C2D_TargetClear(bottom, COLOR_LGRAY);
	C2D_SceneBegin(bottom);
	Toggle_Draw(mirrorToggle, 0);
	Toggle_Draw(explodeToggle, 0);
	Button_Draw(speedUpButton, 0);
	Button_Draw(speedDownButton, 0);
	Button_Draw(saveButton, 0);
	Button_Draw(exitButton, 0);
	Text_Draw(optionsText, DESC_X, OPTION_Y_START + 5, 0, COLOR_DGREEN, 1,
			TEXT_LEFT);
	Text_Draw(speedUpText, BUTTON_X + 87, OPTION_Y_START + OPTION_GAP + 5, 1,
			COLOR_LGRAY, 1, TEXT_LEFT);
	Text_Draw(speedDownText, BUTTON_X + 7, OPTION_Y_START + OPTION_GAP + 5, 1,
			COLOR_LGRAY, 1, TEXT_LEFT);
	Text_Draw(speedText, BUTTON_X + 50, OPTION_Y_START + OPTION_GAP + 5, 1,
			COLOR_DGRAY, 1, TEXT_CENTER);
	Text_Draw(saveText, SAVE_BUTTON_X + 50, SAVE_BUTTON_Y + 5, 1, COLOR_LGRAY,
			1, TEXT_CENTER);
	Text_Draw(exitText, EXIT_BUTTON_X + 24, EXIT_BUTTON_Y + 5, 0.5, COLOR_LGRAY,
			1, TEXT_CENTER);
}

Scene sceneSettings = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

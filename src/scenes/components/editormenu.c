#include <stdbool.h>
#include <citro2d.h>
#include "editormenu.h"
#include "border.h"
#include "button.h"
#include "text.h"
#include "../../rendering/colors.h"
#include "../../rendering/spritesheet.h"
#include "../../util/touchinput.h"
#include "../../util/dispatcher.h"

#define MENU_X 200
#define MENU_Y (BORDER_WIDTH + 20)
#define MENU_WIDTH 120
#define MENU_HEIGHT (240 - MENU_Y - BORDER_WIDTH - 10)
#define MENU_BUTTON_GAP 35
#define MENU_BUTTON_X (MENU_X + 10)
#define MENU_BUTTON_Y (MENU_Y + 10)

static bool isMenuOpen;
static Button showButton, hideButton;

static Button editNameButton, saveButton, exitButton, parUpButton, parDownButton;
static Text   editNameText,   saveText,   exitText,   parUpText,   parDownText;

static void toggleMenu(void *ignored) {
	isMenuOpen = !isMenuOpen;
	if (isMenuOpen) {
		Button_Disable(showButton);
		Button_Enable(hideButton);
		Button_Enable(editNameButton);
		Button_Enable(saveButton);
		Button_Enable(exitButton);
		Button_Enable(parUpButton);
		Button_Enable(parDownButton);
	} else {
		Button_Disable(hideButton);
		Button_Enable(showButton);
		Button_Disable(editNameButton);
		Button_Disable(saveButton);
		Button_Disable(exitButton);
		Button_Disable(parUpButton);
		Button_Disable(parDownButton);
	}
}

bool EditorMenu_Init(
		void (*editName)(void *ignored),
		void (*saveExit)(void *ignored),
		void (*exitNoSave)(void *ignored),
		void (*changePar)(int change)) {
	showButton = Button_Create(308, MENU_Y, SPRITE_BUTTON_LEFT, NULL,
			toggleMenu);
	if (!showButton) goto f_showButton;

	hideButton = Button_Create(MENU_X - BORDER_WIDTH - 11, MENU_Y,
			SPRITE_BUTTON_RIGHT, NULL, toggleMenu);
	if (!hideButton) goto f_hideButton;

	editNameButton = Button_Create(MENU_BUTTON_X, MENU_BUTTON_Y,
			SPRITE_MEDIUM_BUTTON, NULL, editName);
	if (!editNameButton) goto f_editNameButton;
	Button_Disable(editNameButton);

	editNameText = Text_Create(16);
	if (!editNameText) goto f_editNameText;
	Text_SetContent(editNameText, "Edit Name");

	saveButton = Button_Create(MENU_BUTTON_X, MENU_BUTTON_Y + MENU_BUTTON_GAP,
			SPRITE_MEDIUM_BUTTON, NULL, saveExit);
	if (!saveButton) goto f_saveButton;
	Button_Disable(saveButton);

	saveText = Text_Create(16);
	if (!saveText) goto f_saveText;
	Text_SetContent(saveText, "Save & Exit");

	exitButton = Button_Create(MENU_BUTTON_X, MENU_BUTTON_Y + 2*MENU_BUTTON_GAP,
			SPRITE_MEDIUM_BUTTON, NULL, exitNoSave);
	if (!exitButton) goto f_exitButton;
	Button_Disable(exitButton);

	exitText = Text_Create(16);
	if (!exitText) goto f_exitText;
	Text_SetContent(exitText, "Exit");

	parUpButton = Button_Create(MENU_X + MENU_WIDTH - 58,
			MENU_BUTTON_Y + 3*MENU_BUTTON_GAP + 15,
			SPRITE_SMALL_BUTTON, (void*)1, (void(*)(void*))changePar);
	if (!parUpButton) goto f_parUpButton;
	Button_Disable(parUpButton);

	parUpText = Text_Create(2);
	if (!parUpText) goto f_parUpText;
	Text_SetContent(parUpText, "+");

	parDownButton = Button_Create(MENU_BUTTON_X,
			MENU_BUTTON_Y + 3*MENU_BUTTON_GAP + 15,
			SPRITE_SMALL_BUTTON, (void*)-1, (void(*)(void*))changePar);
	if (!parDownButton) goto f_parDownButton;
	Button_Disable(parDownButton);

	parDownText = Text_Create(2);
	if (!parDownText) goto f_parDownText;
	Text_SetContent(parDownText, "-");

	isMenuOpen = false;

	return true;

f_parDownText:
	Button_Free(parDownButton);
f_parDownButton:
	Text_Free(parUpText);
f_parUpText:
	Button_Free(parUpButton);
f_parUpButton:
	Text_Free(exitText);
f_exitText:
	Button_Free(exitButton);
f_exitButton:
	Text_Free(saveText);
f_saveText:
	Button_Free(saveButton);
f_saveButton:
	Text_Free(editNameText);
f_editNameText:
	Button_Free(editNameButton);
f_editNameButton:
	Button_Free(hideButton);
f_hideButton:
	Button_Free(showButton);
f_showButton:
	return false;
}

void EditorMenu_Exit() {
	Button_Free(showButton);
	Button_Free(hideButton);
	Button_Free(editNameButton);
	Button_Free(saveButton);
	Button_Free(exitButton);
	Text_Free(editNameText);
	Text_Free(saveText);
	Text_Free(exitText);
	Button_Free(parUpButton);
	Button_Free(parDownButton);
	Text_Free(parUpText);
	Text_Free(parDownText);
}

// ignored param is to match the signature of Dispatcher_Handler
static bool handleBackgroundTouch(void *ignored) {
	if (!isMenuOpen) return false;

	if (TouchInput_InProgress()) {
		TouchInput_Swipe touch = TouchInput_GetSwipe();
		if (touch.start.px >= MENU_X - BORDER_WIDTH
				&& touch.start.px < MENU_X + MENU_WIDTH
					+ BORDER_WIDTH
				&& touch.start.py >= MENU_Y - BORDER_WIDTH
				&& touch.start.py < MENU_Y + MENU_HEIGHT
					+ BORDER_WIDTH) {
			return true;
		}
	}

	return false;
}

bool EditorMenu_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority) {
	if(!Button_RegisterForTouchEvents(showButton, touchDispatcher, priority))
		goto f_showButton;
	if(!Button_RegisterForTouchEvents(hideButton, touchDispatcher, priority))
		goto f_hideButton;
	if(!Button_RegisterForTouchEvents(editNameButton, touchDispatcher, priority))
		goto f_editNameButton;
	if(!Button_RegisterForTouchEvents(saveButton, touchDispatcher, priority))
		goto f_saveButton;
	if(!Button_RegisterForTouchEvents(exitButton, touchDispatcher, priority))
		goto f_exitButton;
	if(!Button_RegisterForTouchEvents(parUpButton, touchDispatcher, priority))
		goto f_parUpButton;
	if(!Button_RegisterForTouchEvents(parDownButton, touchDispatcher, priority))
		goto f_parDownButton;
	if(!Dispatcher_AddHandler(touchDispatcher, (Dispatcher_Handler) {
			priority, NULL, handleBackgroundTouch }))
		goto f_handleBackgroundTouch;

	return true;

f_handleBackgroundTouch:
	Button_RemoveFromTouchDispatcher(parDownButton, touchDispatcher);
f_parDownButton:
	Button_RemoveFromTouchDispatcher(parUpButton, touchDispatcher);
f_parUpButton:
	Button_RemoveFromTouchDispatcher(exitButton, touchDispatcher);
f_exitButton:
	Button_RemoveFromTouchDispatcher(saveButton, touchDispatcher);
f_saveButton:
	Button_RemoveFromTouchDispatcher(editNameButton, touchDispatcher);
f_editNameButton:
	Button_RemoveFromTouchDispatcher(hideButton, touchDispatcher);
f_hideButton:
	Button_RemoveFromTouchDispatcher(showButton, touchDispatcher);
f_showButton:
	return false;
}

void EditorMenu_RemoveFromTouchDispatcher(Dispatcher touchDispatcher) {
	Button_RemoveFromTouchDispatcher(showButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(hideButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(editNameButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(saveButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(exitButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(parUpButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(parDownButton, touchDispatcher);
	Dispatcher_RemoveHandler(touchDispatcher, (Dispatcher_Handler) {
			0, NULL, handleBackgroundTouch });
}

void EditorMenu_Draw(float depth) {
	float layer1 = depth,
		layer2 = nextafterf(layer1, 0),
		layer3 = nextafterf(layer2, 0);
	Button_Draw(showButton, layer1);
	if (!isMenuOpen) return;

	Button_Draw(hideButton, layer1);
	Border_Draw(MENU_X, MENU_Y, layer3, MENU_WIDTH, MENU_HEIGHT);
	C2D_DrawRectSolid(MENU_X, MENU_Y, layer3, MENU_WIDTH, MENU_HEIGHT,
			COLOR_LGRAY);

	Button_Draw(editNameButton, layer2);
	Button_Draw(saveButton, layer2);
	Button_Draw(exitButton, layer2);
	Text_Draw(
			editNameText,
			MENU_BUTTON_X + 10,
			MENU_BUTTON_Y + 5,
			layer1, COLOR_LGRAY, 1
		);
	Text_Draw(
			saveText,
			MENU_BUTTON_X + 10,
			MENU_BUTTON_Y + MENU_BUTTON_GAP + 5,
			layer1, COLOR_LGRAY, 1
		);
	Text_Draw(
			exitText,
			MENU_BUTTON_X + 10,
			MENU_BUTTON_Y + 2*MENU_BUTTON_GAP + 5,
			layer1, COLOR_LGRAY, 1
		);

	C2D_DrawImageAt(
			SpriteSheet_GetImage(SPRITE_PAR_LABEL),
			MENU_BUTTON_X + 2,
			MENU_BUTTON_Y + 3*MENU_BUTTON_GAP - 1,
			layer1,
			NULL, 1, 1
		);
	Button_Draw(parUpButton, layer2);
	Button_Draw(parDownButton, layer2);
	Text_Draw(
			parUpText,
			MENU_X + MENU_WIDTH - 40,
			MENU_BUTTON_Y + 3*MENU_BUTTON_GAP + 10,
			layer1, COLOR_LGRAY, 2
		);
	Text_Draw(
			parDownText,
			MENU_BUTTON_X + 18,
			MENU_BUTTON_Y + 3*MENU_BUTTON_GAP + 10,
			layer1, COLOR_LGRAY, 2
		);
}

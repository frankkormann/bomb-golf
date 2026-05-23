#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <3ds.h>
#include <citro2d.h>
#include "popup.h"
#include "button.h"
#include "text.h"
#include "border.h"
#include "../../rendering/spritesheet.h"
#include "../../rendering/colors.h"
#include "../../util/dispatcher.h"

#define MAX_BUTTONS 2

#define MARGIN_X 20
#define MARGIN_Y 10

#define BUTTON_HEIGHT 30

#define ONE_BUTTON_X 110

#define TWO_BUTTON_START_X 50
#define TWO_BUTTON_GAP 110

static Popup_Format format;
static Text messageText;
static float messageHeight;

static Dispatcher touchDispatcher;
static Text   buttonsText[MAX_BUTTONS];
static Button buttons[MAX_BUTTONS];

static float calculateButtonY(float messageHeight) {
	return 120 - (messageHeight + BUTTON_HEIGHT + 3*MARGIN_Y)/2
			+ messageHeight + 2*MARGIN_Y;
}

bool Popup_Init(char *message, Popup_Format argFormat, Popup_Button argButtons[]) {
	format = argFormat;

	messageText = Text_Create(strlen(message) + 1);
	if (!messageText) goto f_messageText;
	Text_SetContent(messageText, "%s", message);

	messageHeight = Text_CalculateHeight(messageText, 1);

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;

	switch (format) {
		case ONE_BUTTON:
			buttons[0] = Button_Create(
					ONE_BUTTON_X,
					calculateButtonY(messageHeight),
					SPRITE_MEDIUM_BUTTON,
					argButtons[0].onTouchParam,
					argButtons[0].onTouch
				);
			if (!buttons[0]) goto f_buttons;
			Button_RegisterForTouchEvents(buttons[0], touchDispatcher,
					1);
			buttonsText[0] = Text_Create(
					strlen(argButtons[0].label) + 1);
			if (!buttonsText[0]) goto f_buttons;
			Text_SetContent(buttonsText[0], "%s", argButtons[0].label);
			break;
		case TWO_BUTTON:
			for (int i = 0; i < 2; i++) {
				buttons[i] = Button_Create(
						TWO_BUTTON_START_X
							+ i*TWO_BUTTON_GAP,
						calculateButtonY(messageHeight),
						SPRITE_MEDIUM_BUTTON,
						argButtons[i].onTouchParam,
						argButtons[i].onTouch
					);
				if (!buttons[i]) goto f_buttons;
				Button_RegisterForTouchEvents(buttons[i],
						touchDispatcher, 1);
				buttonsText[i] = Text_Create(
						strlen(argButtons[i].label) + 1);
				if (!buttonsText[i]) goto f_buttons;
				Text_SetContent(buttonsText[i], "%s",
						argButtons[i].label);
			}
			break;
	}

	return true;

f_buttons:
	for (int i = 0; i < MAX_BUTTONS; i++) {
		if (buttons[i]) Button_Free(buttons[i]);
		if (buttonsText[i]) Text_Free(buttonsText[i]);
		buttons[i] = NULL;
	}
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	Text_Free(messageText);
f_messageText:
	return false;
}

void Popup_Exit() {
	Text_Free(messageText);
	for (int i = 0; i < MAX_BUTTONS; i++) {
		if (buttons[i]) Button_Free(buttons[i]);
		if (buttonsText[i]) Text_Free(buttonsText[i]);
		buttons[i] = NULL;
	}
	Dispatcher_Free(touchDispatcher);
}

void Popup_Update() {
	Dispatcher_DispatchEvent(touchDispatcher);
}

void Popup_Draw() {
	C2D_DrawRectSolid(0, 0, 1, 320, 240, COLOR_DGRAY & 0x77FFFFFF); 
	float height = messageHeight + 3*MARGIN_Y + BUTTON_HEIGHT;
	float y = 120 - height/2;
	Border_Draw(MARGIN_X, y, 1, 320 - 2*MARGIN_X, height);
	C2D_DrawRectSolid(MARGIN_X, y, 1, 320 - 2*MARGIN_X, height, COLOR_LGRAY);

	Text_Draw(messageText, 2*MARGIN_X, y + MARGIN_Y, 1, COLOR_DGREEN, 1,
			TEXT_LEFT);

	for (int i = 0; i < MAX_BUTTONS; i++) {
		Button_Draw(buttons[i], 1);
		float textX = 0, textY = 0;
		switch (format) {
			case ONE_BUTTON:
				textX = ONE_BUTTON_X + 10;
				textY = calculateButtonY(messageHeight) + 5;
				break;
			case TWO_BUTTON:
				textX = TWO_BUTTON_START_X + i*TWO_BUTTON_GAP + 10;
				textY = calculateButtonY(messageHeight) + 5;
				break;
		}
		Text_Draw(buttonsText[i], textX, textY, 1, COLOR_LGRAY, 1,
				TEXT_LEFT);
	}
}

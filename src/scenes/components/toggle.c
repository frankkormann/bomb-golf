#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include <citro2d.h>
#include "toggle.h"
#include "button.h"
#include "text.h"
#include "../../rendering/spritesheet.h"
#include "../../rendering/color.h"
#include "../../util/dispatcher.h"

struct toggle {
	float x;
	float y;
	Button leftBtn;
	Button rightBtn;
	Text leftText;
	Text rightText;
	int leftVal;
	int rightVal;
	enum { LEFT, RIGHT } selected;
};

static void setLeft(Toggle toggle) {
	toggle->selected = LEFT;
}

static void setRight(Toggle toggle) {
	toggle->selected = RIGHT;
}

Toggle Toggle_Create(float x, float y, const char *leftLabel,
		const char *rightLabel, int leftVal, int rightVal,
		bool isLeftSelected) {
	Toggle toggle = malloc(sizeof(struct toggle));
	if (!toggle) goto f_toggle;

	toggle->x = x;
	toggle->y = y;
	toggle->leftVal = leftVal;
	toggle->rightVal = rightVal;
	toggle->selected = isLeftSelected ? LEFT : RIGHT;

	toggle->leftBtn = Button_Create(x, y, SPRITE_SMALL_BUTTON, -1, toggle,
			(void(*)(void*))setLeft);
	if (!toggle->leftBtn) goto f_leftBtn;

	toggle->rightBtn = Button_Create(x + 52, y, SPRITE_SMALL_BUTTON, -1, toggle,
			(void(*)(void*))setRight);
	if (!toggle->rightBtn) goto f_rightBtn;

	toggle->leftText = Text_Create(strlen(leftLabel) + 1);
	if (!toggle->leftText) goto f_leftText;
	Text_SetContent(toggle->leftText, leftLabel);

	toggle->rightText = Text_Create(strlen(rightLabel) + 1);
	if (!toggle->rightText) goto f_rightText;
	Text_SetContent(toggle->rightText, rightLabel);

	return toggle;

f_rightText:
	Text_Free(toggle->leftText);
f_leftText:
	Button_Free(toggle->rightBtn);
f_rightBtn:
	Button_Free(toggle->leftBtn);
f_leftBtn:
	free(toggle);
f_toggle:
	return NULL;
}

void Toggle_Free(Toggle toggle) {
	Button_Free(toggle->leftBtn);
	Button_Free(toggle->rightBtn);
	Text_Free(toggle->leftText);
	Text_Free(toggle->rightText);
	free(toggle);
}

int Toggle_GetValue(Toggle toggle) {
	return toggle->selected == LEFT ? toggle->leftVal : toggle->rightVal;
}

bool Toggle_RegisterForTouchEvents(Toggle toggle, Dispatcher touchDispatcher,
		int priority) {
	if (!Button_RegisterForTouchEvents(toggle->leftBtn, touchDispatcher,
			priority)) return false;
	if (!Button_RegisterForTouchEvents(toggle->rightBtn, touchDispatcher,
			priority)) {
		Button_RemoveFromTouchDispatcher(toggle->leftBtn, touchDispatcher);
		return false;
	}
	return true;
}

void Toggle_RemoveFromTouchDispatcher(Toggle toggle, Dispatcher touchDispatcher) {
	Button_RemoveFromTouchDispatcher(toggle->leftBtn, touchDispatcher);
	Button_RemoveFromTouchDispatcher(toggle->rightBtn, touchDispatcher);
}

static void drawOutline(int x, int y, float depth, int width, int height,
		u32 color, int outlineWidth) {
	C2D_DrawRectSolid(x+1, y, depth, width-2, outlineWidth, color);
	C2D_DrawRectSolid(x, y+1, depth, outlineWidth, height-2, color);
	C2D_DrawRectSolid(x+1, y + height - outlineWidth, 0, width-2, outlineWidth,
			color);
	C2D_DrawRectSolid(x + width - outlineWidth, y+1, 0, outlineWidth, height-2,
			color);
}

void Toggle_Draw(Toggle toggle, float depth) {
	Button_Draw(toggle->leftBtn, depth);
	Button_Draw(toggle->rightBtn, depth);
	if (toggle->selected == LEFT) {
		drawOutline(toggle->x, toggle->y, depth, 48, 30, COLOR_DRED, 2);
	} else {
		drawOutline(toggle->x + 52, toggle->y, depth, 48, 30, COLOR_DRED, 2);
	}
	Text_Draw(toggle->leftText, toggle->x + 24, toggle->y + 5, depth,
			COLOR_LGRAY, 1, TEXT_CENTER);
	Text_Draw(toggle->rightText, toggle->x + 76, toggle->y + 5, depth,
			COLOR_LGRAY, 1, TEXT_CENTER);
}

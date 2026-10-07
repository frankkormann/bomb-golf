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

#define BUTTON_GAP 52

typedef struct {
	Toggle parent;
	int index;
} ButtonParam;

struct toggle {
	float x;
	float y;
	Button *btns;
	Text *labels;
	ButtonParam *btnParams;
	int *values;
	int numBtns;
	int selected;
};

static void updateToggle(void *param) {
	ButtonParam *btnParam = param;
	btnParam->parent->selected = btnParam->index;
}

Toggle Toggle_Create(float x, float y, int numButtons, int initIndex,
		Toggle_Button buttons[]) {
	Toggle toggle = malloc(sizeof(struct toggle));
	if (!toggle) goto f_toggle;

	toggle->x = x;
	toggle->y = y;

	toggle->btns = calloc(numButtons, sizeof(*toggle->btns));
	if (!toggle->btns) goto f_btns;
	toggle->labels = calloc(numButtons, sizeof(*toggle->labels));
	if (!toggle->labels) goto f_labels;
	toggle->btnParams = calloc(numButtons, sizeof(*toggle->btnParams));
	if (!toggle->btnParams) goto f_btnParams;
	toggle->values = calloc(numButtons, sizeof(*toggle->values));
	if (!toggle->values) goto f_values;

	for (int i = 0; i < numButtons; i++) {
		toggle->btns[i] = Button_Create(x + i*BUTTON_GAP, y,
				SPRITE_SMALL_BUTTON, -1, toggle->btnParams + i,
				updateToggle);
		if (!toggle->btns[i]) goto f_things;

		toggle->labels[i] = Text_Create(strlen(buttons[i].label) + 1);
		if (!toggle->labels[i]) goto f_things;
		Text_SetContent(toggle->labels[i], buttons[i].label);

		toggle->btnParams[i].parent = toggle;
		toggle->btnParams[i].index = i;
		toggle->values[i] = buttons[i].value;
	}

	toggle->numBtns = numButtons;
	toggle->selected = initIndex;

	return toggle;

f_things:
	for (int i = 0; i < numButtons; i++) {
		if (toggle->btns[i]) Button_Free(toggle->btns[i]);
		if (toggle->labels[i]) Text_Free(toggle->labels[i]);
	}
	free(toggle->values);
f_values:
	free(toggle->btnParams);
f_btnParams:
	free(toggle->labels);
f_labels:
	free(toggle->btns);
f_btns:
	free(toggle);
f_toggle:
	return NULL;
}

void Toggle_Free(Toggle toggle) {
	for (int i = 0; i < toggle->numBtns; i++) {
		Button_Free(toggle->btns[i]);
		Text_Free(toggle->labels[i]);
	}
	free(toggle->values);
	free(toggle->btnParams);
	free(toggle->labels);
	free(toggle->btns);
	free(toggle);
}

int Toggle_GetValue(Toggle toggle) {
	return toggle->values[toggle->selected];
}

bool Toggle_RegisterForTouchEvents(Toggle toggle, Dispatcher touchDispatcher,
		int priority) {
	for (int i = 0; i < toggle->numBtns; i++) {
		if (!Button_RegisterForTouchEvents(toggle->btns[i], touchDispatcher,
				priority)) {
			for (int j = 0; j < i; j++) {
				Button_RemoveFromTouchDispatcher(toggle->btns[j],
						touchDispatcher);
			}
			return false;
		}
	}
	return true;
}

void Toggle_RemoveFromTouchDispatcher(Toggle toggle, Dispatcher touchDispatcher) {
	for (int i = 0; i < toggle->numBtns; i++) {
		Button_RemoveFromTouchDispatcher(toggle->btns[i], touchDispatcher);
	}
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
	for (int i = 0; i < toggle->numBtns; i++) {
		Button_Draw(toggle->btns[i], depth);
		Text_Draw(toggle->labels[i], toggle->x + i*BUTTON_GAP + 24,
				toggle->y + 5, depth, COLOR_LGRAY, 1, TEXT_CENTER);
	}
	drawOutline(toggle->x + BUTTON_GAP * toggle->selected, toggle->y, depth, 48,
			30, COLOR_DRED, 2);
}

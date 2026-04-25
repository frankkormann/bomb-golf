#include <stdbool.h>
#include <citro2d.h>
#include "brushselector.h"
#include "button.h"
#include "../../rendering/spritesheet.h"
#include "../../rendering/colors.h"
#include "../../util/dispatcher.h"

#define BUTTON_START_Y 43
#define BUTTON_X 1
#define BUTTON_GAP 13

static bool isOpen;
static Button expandButton, shrinkButton;

static BrushSelector_Brush brush;
static Button pencilButton, rectangleButton, ballButton, holeButton;

static void toggleOpen(void *ignored) {
	isOpen = !isOpen;
	if (isOpen) {
		Button_Enable(shrinkButton);
		Button_Enable(pencilButton);
		Button_Enable(rectangleButton);
		Button_Enable(ballButton);
		Button_Enable(holeButton);
		Button_Disable(expandButton);
	} else {
		Button_Enable(expandButton);
		Button_Disable(shrinkButton);
		Button_Disable(pencilButton);
		Button_Disable(rectangleButton);
		Button_Disable(ballButton);
		Button_Disable(holeButton);
	}
}

static void setBrush(BrushSelector_Brush newBrush) {
	brush = newBrush;
}

bool BrushSelector_Init(BrushSelector_Brush defaultBrush) {
	expandButton = Button_Create(3, BUTTON_START_Y - 18, SPRITE_BUTTON_RIGHT,
			NULL, toggleOpen);
	if (!expandButton) goto f_expandButton;
	Button_Disable(expandButton);

	shrinkButton = Button_Create(3, BUTTON_START_Y - 18, SPRITE_BUTTON_LEFT,
			NULL, toggleOpen);
	if (!shrinkButton) goto f_shrinkButton;

	pencilButton = Button_Create(BUTTON_X, BUTTON_START_Y,
			SPRITE_PENCIL_BUTTON,
			(void*)BRUSH_PENCIL, (void(*)(void*))setBrush);
	if (!pencilButton) goto f_pencilButton;

	rectangleButton = Button_Create(BUTTON_X, BUTTON_START_Y + BUTTON_GAP,
			SPRITE_RECTANGLE_BUTTON,
			(void*)BRUSH_RECTANGLE, (void(*)(void*))setBrush);
	if (!rectangleButton) goto f_rectangleButton;

	ballButton = Button_Create(BUTTON_X, BUTTON_START_Y + 2*BUTTON_GAP,
			SPRITE_BALL_BUTTON,
			(void*)BRUSH_BALL_POS, (void(*)(void*))setBrush);
	if (!ballButton) goto f_ballButton;

	holeButton = Button_Create(BUTTON_X, BUTTON_START_Y + 3*BUTTON_GAP,
			SPRITE_HOLE_BUTTON,
			(void*)BRUSH_HOLE_POS, (void(*)(void*))setBrush);
	if (!holeButton) goto f_holeButton;

	isOpen = true;

	return true;

f_holeButton:
	Button_Free(ballButton);
f_ballButton:
	Button_Free(rectangleButton);
f_rectangleButton:
	Button_Free(pencilButton);
f_pencilButton:
	Button_Free(shrinkButton);
f_shrinkButton:
	Button_Free(expandButton);
f_expandButton:
	return false;
}

void BrushSelector_Exit() {
	Button_Free(expandButton);
	Button_Free(shrinkButton);
	Button_Free(pencilButton);
	Button_Free(rectangleButton);
	Button_Free(ballButton);
	Button_Free(holeButton);
}

bool BrushSelector_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority) {
	if (!Button_RegisterForTouchEvents(expandButton, touchDispatcher, priority))
		goto f_expandButton;
	if (!Button_RegisterForTouchEvents(shrinkButton, touchDispatcher, priority))
		goto f_shrinkButton;
	if (!Button_RegisterForTouchEvents(pencilButton, touchDispatcher, priority))
		goto f_pencilButton;
	if (!Button_RegisterForTouchEvents(rectangleButton, touchDispatcher,
			priority))
		goto f_rectangleButton;
	if (!Button_RegisterForTouchEvents(ballButton, touchDispatcher, priority))
		goto f_ballButton;
	if (!Button_RegisterForTouchEvents(holeButton, touchDispatcher, priority))
		goto f_holeButton;

	return true;

f_holeButton:
	Button_RemoveFromTouchDispatcher(ballButton, touchDispatcher);
f_ballButton:
	Button_RemoveFromTouchDispatcher(rectangleButton, touchDispatcher);
f_rectangleButton:
	Button_RemoveFromTouchDispatcher(pencilButton, touchDispatcher);
f_pencilButton:
	Button_RemoveFromTouchDispatcher(shrinkButton, touchDispatcher);
f_shrinkButton:
	Button_RemoveFromTouchDispatcher(expandButton, touchDispatcher);
f_expandButton:
	return false;
}

void BrushSelector_RemoveFromTouchDispatcher(Dispatcher touchDispatcher) {
	Button_RemoveFromTouchDispatcher(pencilButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(rectangleButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(ballButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(holeButton, touchDispatcher);
}

BrushSelector_Brush BrushSelector_GetBrush() {
	return brush;
}

static void drawRectOutline(float x, float y, float depth, float width, float height,
		u32 color, int outlineWidth) {
	C2D_DrawRectSolid(x, y, depth, width, outlineWidth, color);
	C2D_DrawRectSolid(x, y, depth, outlineWidth, height, color);
	C2D_DrawRectSolid(x, y + height - outlineWidth, depth, width, outlineWidth,
			color);
	C2D_DrawRectSolid(x + width - outlineWidth, y, depth, outlineWidth, height,
			color);
}

void BrushSelector_Draw(float depth) {
	Button_Draw(expandButton, depth);
	Button_Draw(shrinkButton, depth);

	if (!isOpen) return;

	Button_Draw(pencilButton, depth);
	Button_Draw(rectangleButton, depth);
	Button_Draw(ballButton, depth);
	Button_Draw(holeButton, depth);
	drawRectOutline(BUTTON_X, BUTTON_START_Y + (BUTTON_GAP * brush),
			depth, 14, 14, COLOR_DRED, 1);
}

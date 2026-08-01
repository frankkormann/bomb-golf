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
#define SECTION_GAP 3

static bool isOpen;
static Button expandButton, shrinkButton;

static BrushSelector_Brush brush;
static Button pencilButton, rectangleButton, ballButton, holeButton, obstAddButton,
		obstEditButton, obstDelButton, obstMoveButton, obstDupeButton;
static int buttonYs[NUM_BRUSHES];

static void toggleOpen() {
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
	expandButton = Button_Create(3, BUTTON_START_Y - 18, SPRITE_BUTTON_RIGHT, -1,
			NULL, toggleOpen);
	if (!expandButton) goto f_expandButton;
	Button_Disable(expandButton);

	shrinkButton = Button_Create(3, BUTTON_START_Y - 18, SPRITE_BUTTON_LEFT, -1,
			NULL, toggleOpen);
	if (!shrinkButton) goto f_shrinkButton;

	int i = 0;
	buttonYs[i] = BUTTON_START_Y;
	pencilButton = Button_Create(BUTTON_X, buttonYs[i],
			SPRITE_PENCIL_BUTTON, -1,
			(void*)BRUSH_PENCIL, (void(*)(void*))setBrush);
	if (!pencilButton) goto f_pencilButton;

	i++;
	buttonYs[i] = buttonYs[i-1] + BUTTON_GAP;
	rectangleButton = Button_Create(BUTTON_X, buttonYs[i],
			SPRITE_RECTANGLE_BUTTON, -1,
			(void*)BRUSH_RECTANGLE, (void(*)(void*))setBrush);
	if (!rectangleButton) goto f_rectangleButton;

	i++;
	buttonYs[i] = buttonYs[i-1] + BUTTON_GAP + SECTION_GAP;
	ballButton = Button_Create(BUTTON_X, buttonYs[i],
			SPRITE_BALL_BUTTON, -1,
			(void*)BRUSH_BALL_POS, (void(*)(void*))setBrush);
	if (!ballButton) goto f_ballButton;

	i++;
	buttonYs[i] = buttonYs[i-1] + BUTTON_GAP;
	holeButton = Button_Create(BUTTON_X, buttonYs[i],
			SPRITE_HOLE_BUTTON, -1,
			(void*)BRUSH_HOLE_POS, (void(*)(void*))setBrush);
	if (!holeButton) goto f_holeButton;

	i++;
	buttonYs[i] = buttonYs[i-1] + BUTTON_GAP + SECTION_GAP;
	obstAddButton = Button_Create(BUTTON_X, buttonYs[i],
			SPRITE_BIRD_BUTTON, -1,
			(void*)BRUSH_OBSTACLE_ADD, (void(*)(void*))setBrush);
	if (!obstAddButton) goto f_obstAddButton;

	i++;
	buttonYs[i] = buttonYs[i-1] + BUTTON_GAP;
	obstDelButton = Button_Create(BUTTON_X, buttonYs[i],
			SPRITE_X_BUTTON, -1,
			(void*)BRUSH_OBSTACLE_DEL, (void(*)(void*))setBrush);
	if (!obstDelButton) goto f_obstDelButton;

	i++;
	buttonYs[i] = buttonYs[i-1] + BUTTON_GAP;
	obstEditButton = Button_Create(BUTTON_X, buttonYs[i],
			SPRITE_WRENCH_BUTTON, -1,
			(void*)BRUSH_OBSTACLE_EDIT, (void(*)(void*))setBrush);
	if (!obstEditButton) goto f_obstEditButton;

	i++;
	buttonYs[i] = buttonYs[i-1] + BUTTON_GAP;
	obstMoveButton = Button_Create(BUTTON_X, buttonYs[i],
			SPRITE_HAND_BUTTON, -1,
			(void*)BRUSH_OBSTACLE_MOVE, (void(*)(void*))setBrush);
	if (!obstMoveButton) goto f_obstMoveButton;

	i++;
	buttonYs[i] = buttonYs[i-1] + BUTTON_GAP;
	obstDupeButton = Button_Create(BUTTON_X, buttonYs[i],
			SPRITE_DUPE_BUTTON, -1,
			(void*)BRUSH_OBSTACLE_DUPE, (void(*)(void*))setBrush);
	if (!obstDupeButton) goto f_obstDupeButton;

	isOpen = true;
	brush = defaultBrush;

	return true;

f_obstDupeButton:
	Button_Free(obstMoveButton);
f_obstMoveButton:
	Button_Free(obstDelButton);
f_obstDelButton:
	Button_Free(obstEditButton);
f_obstEditButton:
	Button_Free(obstAddButton);
f_obstAddButton:
	Button_Free(holeButton);
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
	Button_Free(obstAddButton);
	Button_Free(obstEditButton);
	Button_Free(obstDelButton);
	Button_Free(obstMoveButton);
	Button_Free(obstDupeButton);
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
	if (!Button_RegisterForTouchEvents(obstAddButton, touchDispatcher, priority))
		goto f_obstAddButton;
	if (!Button_RegisterForTouchEvents(obstEditButton, touchDispatcher,
			priority))
		goto f_obstEditButton;
	if (!Button_RegisterForTouchEvents(obstDelButton, touchDispatcher,
			priority))
		goto f_obstDelButton;
	if (!Button_RegisterForTouchEvents(obstMoveButton, touchDispatcher,
			priority))
		goto f_obstMoveButton;
	if (!Button_RegisterForTouchEvents(obstDupeButton, touchDispatcher,
			priority))
		goto f_obstDupeButton;

	return true;

f_obstDupeButton:
	Button_RemoveFromTouchDispatcher(obstMoveButton, touchDispatcher);
f_obstMoveButton:
	Button_RemoveFromTouchDispatcher(obstDelButton, touchDispatcher);
f_obstDelButton:
	Button_RemoveFromTouchDispatcher(obstEditButton, touchDispatcher);
f_obstEditButton:
	Button_RemoveFromTouchDispatcher(obstAddButton, touchDispatcher);
f_obstAddButton:
	Button_RemoveFromTouchDispatcher(holeButton, touchDispatcher);
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
	Button_RemoveFromTouchDispatcher(obstAddButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(obstEditButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(obstDelButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(obstMoveButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(obstDupeButton, touchDispatcher);
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
	Button_Draw(obstAddButton, depth);
	Button_Draw(obstEditButton, depth);
	Button_Draw(obstDelButton, depth);
	Button_Draw(obstMoveButton, depth);
	Button_Draw(obstDupeButton, depth);
	drawRectOutline(BUTTON_X, buttonYs[brush], depth, 14, 14, COLOR_DRED, 1);
}

#include <stdbool.h>
#include <malloc.h>
#include <citro2d.h>
#include "obstacleeditor.h"
#include "text.h"
#include "button.h"
#include "border.h"
#include "../../rendering/spritesheet.h"
#include "../../rendering/colors.h"
#include "../../environment/obstacle.h"
#include "../../util/dispatcher.h"
#include "../../util/macros.h"
#include "../../util/touchinput.h"

#define BOX_X (SPRITE_LEFT_X - 10)
#define BOX_Y (SPRITE_RIGHT_Y - 10)
#define BOX_WIDTH 120
#define BOX_HEIGHT (EXIT_Y - BOX_Y + 40)
#define SPRITE_X (160 - SPRITE_WIDTH/2)
#define SPRITE_Y 60
#define SPRITE_WIDTH 60
#define SPRITE_HEIGHT 40
#define SPRITE_LEFT_X (SPRITE_X - 20)
#define SPRITE_LEFT_Y (SPRITE_Y + SPRITE_HEIGHT/2 - 15)
#define SPRITE_RIGHT_X (SPRITE_X + SPRITE_WIDTH)
#define SPRITE_RIGHT_Y (SPRITE_Y + SPRITE_HEIGHT/2 - 15)
#define SPEED_TEXT_X 160
#define SPEED_TEXT_Y (SPRITE_Y + SPRITE_HEIGHT + 10)
#define SPEED_UP_X SPRITE_RIGHT_X
#define SPEED_UP_Y (SPEED_TEXT_Y - 5)
#define SPEED_DOWN_X SPRITE_LEFT_X
#define SPEED_DOWN_Y (SPEED_TEXT_Y - 5)
/*
#define PATH_X (160 - 50)
#define PATH_Y (SPEED_UP_Y + 35)
*/
#define EXIT_X (160 - 50)
#define EXIT_Y (SPEED_UP_Y + 35)

static Button exitButton, speedUpButton, speedDownButton, spriteUpButton,
		spriteDownButton /*, pathButton */;
static Text exitText, upText, downText, leftText, rightText, /* pathText, */ speedText;

static bool isShowing;
//static List obstacleList;
static Obstacle_Data *curObst;

// Declarations needed for buttons, dispatcher
static void hideEditor();
static void changeSprite(bool goUp);
static void changeSpeed(int change);
//static void editPath();
static bool handleTouchInput();

bool ObstacleEditor_Init() {
	exitButton = Button_Create(EXIT_X, EXIT_Y, SPRITE_MEDIUM_BUTTON, -1, NULL,
			hideEditor);
	if (!exitButton) goto f_exitButton;
	Button_Disable(exitButton);

	speedUpButton = Button_Create(SPEED_UP_X, SPEED_UP_Y,
			SPRITE_THIN_BUTTON, -1,
			(void*)1, (void(*)(void*))changeSpeed);
	if (!speedUpButton) goto f_speedUpButton;
	Button_Disable(speedUpButton);

	speedDownButton = Button_Create(SPEED_DOWN_X, SPEED_DOWN_Y,
			SPRITE_THIN_BUTTON, -1,
			(void*)-1, (void(*)(void*))changeSpeed);
	if (!speedDownButton) goto f_speedDownButton;
	Button_Disable(speedDownButton);

	spriteUpButton = Button_Create(SPRITE_RIGHT_X, SPRITE_RIGHT_Y,
			SPRITE_THIN_BUTTON, -1,
			(void*)true, (void(*)(void*))changeSprite);
	if (!spriteUpButton) goto f_spriteUpButton;
	Button_Disable(spriteUpButton);

	spriteDownButton = Button_Create(SPRITE_LEFT_X, SPRITE_LEFT_Y,
			SPRITE_THIN_BUTTON, -1,
			(void*)false, (void(*)(void*))changeSprite);
	if (!spriteDownButton) goto f_spriteDownButton;
	Button_Disable(spriteDownButton);

	exitText = Text_Create(8);
	if (!exitText) goto f_exitText;
	Text_SetContent(exitText, "Done");

	upText = Text_Create(2);
	if (!upText) goto f_upText;
	Text_SetContent(upText, "+");

	downText = Text_Create(2);
	if (!downText) goto f_downText;
	Text_SetContent(downText, "-");

	leftText = Text_Create(2);
	if (!leftText) goto f_leftText;
	Text_SetContent(leftText, "<");

	rightText = Text_Create(2);
	if (!rightText) goto f_rightText;
	Text_SetContent(rightText, ">");

	speedText = Text_Create(9);
	if (!speedText) goto f_speedText;

	isShowing = false;

	return true;

f_speedText:
	Text_Free(rightText);
f_rightText:
	Text_Free(leftText);
f_leftText:
	Text_Free(downText);
f_downText:
	Text_Free(upText);
f_upText:
	Text_Free(exitText);
f_exitText:
	Button_Free(spriteDownButton);
f_spriteDownButton:
	Button_Free(spriteUpButton);
f_spriteUpButton:
	Button_Free(speedDownButton);
f_speedDownButton:
	Button_Free(speedUpButton);
f_speedUpButton:
	Button_Free(exitButton);
f_exitButton:
	return false;
}

void ObstacleEditor_Exit() {
	Button_Free(spriteDownButton);
	Button_Free(spriteUpButton);
	Button_Free(speedDownButton);
	Button_Free(speedUpButton);
	Button_Free(exitButton);
	Text_Free(downText);
	Text_Free(upText);
	Text_Free(leftText);
	Text_Free(rightText);
	Text_Free(exitText);
	Text_Free(speedText);
}

void ObstacleEditor_Show(Obstacle_Data *toEdit) {
	if (isShowing) return;

	isShowing = true;
	curObst = toEdit;
	changeSpeed(0);  // Get initial text in speedText
	Button_Enable(spriteDownButton);
	Button_Enable(spriteUpButton);
	Button_Enable(speedDownButton);
	Button_Enable(speedUpButton);
	Button_Enable(exitButton);
}

static bool handleTouchInput() {
	// If this is a popup, consume all touch events that weren't handled
	// by a button
	return (TouchInput_InProgress() || TouchInput_JustFinished()) && isShowing;
}

static void hideEditor() {
	isShowing = !isShowing;
	Button_Disable(spriteDownButton);
	Button_Disable(spriteUpButton);
	Button_Disable(speedDownButton);
	Button_Disable(speedUpButton);
	Button_Disable(exitButton);
}

static void changeSprite(bool goUp) {
	curObst->sprite1 += goUp ? 2 : -2;
	curObst->sprite1 = (curObst->sprite1 + NUM_OBSTS) % NUM_OBSTS;
	curObst->sprite2 = curObst->sprite1 + 1;
}

// Change in units of 1/4 so that change can be an int for callback signature
static void changeSpeed(int change) {
	curObst->speed += (float)change/4;
	curObst->speed = clamp(curObst->speed, 0, 10);
	Text_SetContent(speedText, "SPD %.2f", curObst->speed);
}

bool ObstacleEditor_RegisterForTouchEvents(Dispatcher touchDispatcher,
		int priority) {
	if (!Button_RegisterForTouchEvents(spriteDownButton, touchDispatcher,
			priority))
		goto f_spriteDownButton;
	if (!Button_RegisterForTouchEvents(spriteUpButton, touchDispatcher,
			priority))
		goto f_spriteUpButton;
	if (!Button_RegisterForTouchEvents(speedDownButton, touchDispatcher,
			priority))
		goto f_speedDownButton;
	if (!Button_RegisterForTouchEvents(speedUpButton, touchDispatcher, priority))
		goto f_speedUpButton;
	if (!Button_RegisterForTouchEvents(exitButton, touchDispatcher, priority))
		goto f_exitButton;
	if (!Dispatcher_AddHandler(touchDispatcher, (Dispatcher_Handler) {
			.priority = priority, NULL, handleTouchInput }))
		goto f_handleTouchInput;

	return true;

f_handleTouchInput:
	Button_RemoveFromTouchDispatcher(exitButton, touchDispatcher);
f_exitButton:
	Button_RemoveFromTouchDispatcher(speedUpButton, touchDispatcher);
f_speedUpButton:
	Button_RemoveFromTouchDispatcher(speedDownButton, touchDispatcher);
f_speedDownButton:
	Button_RemoveFromTouchDispatcher(spriteUpButton, touchDispatcher);
f_spriteUpButton:
	Button_RemoveFromTouchDispatcher(spriteDownButton, touchDispatcher);
f_spriteDownButton:
	return false;
}

void ObstacleEditor_RemoveFromTouchDispatcher(Dispatcher touchDispatcher) {
	Button_RemoveFromTouchDispatcher(exitButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(speedUpButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(speedDownButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(spriteUpButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(spriteDownButton, touchDispatcher);
	Dispatcher_RemoveHandler(touchDispatcher, (Dispatcher_Handler) {
			.priority = 0, NULL, handleTouchInput });
}

void ObstacleEditor_Draw(float depth) {
	if (!isShowing) return;

	float below = nextafter(depth, -1);

	Border_Draw(BOX_X, BOX_Y, below, BOX_WIDTH, BOX_HEIGHT);
	C2D_DrawRectSolid(BOX_X, BOX_Y, below, BOX_WIDTH, BOX_HEIGHT, COLOR_LGRAY);

	SpriteSheet_DrawObstacle(curObst->sprite1, SPRITE_X + SPRITE_WIDTH/2,
			SPRITE_Y + SPRITE_HEIGHT/2, depth, 0, false, false);

	Button_Draw(exitButton, depth);
	Button_Draw(speedUpButton, depth);
	Button_Draw(speedDownButton, depth);
	Button_Draw(spriteUpButton, depth);
	Button_Draw(spriteDownButton, depth);

	Text_Draw(speedText, SPEED_TEXT_X, SPEED_TEXT_Y, depth, COLOR_DGRAY, 1,
			TEXT_CENTERED);
	Text_Draw(exitText, EXIT_X + 10, EXIT_Y + 5, depth, COLOR_LGRAY, 1,
			TEXT_LEFT);
	Text_Draw(rightText, SPRITE_RIGHT_X + 7, SPRITE_RIGHT_Y + 5, depth,
			COLOR_LGRAY, 1, TEXT_LEFT);
	Text_Draw(leftText, SPRITE_LEFT_X + 7, SPRITE_LEFT_Y + 5, depth,
			COLOR_LGRAY, 1, TEXT_LEFT);
	Text_Draw(upText, SPEED_UP_X + 7, SPEED_UP_Y + 5, depth, COLOR_LGRAY,
			1, TEXT_LEFT);
	Text_Draw(downText, SPEED_DOWN_X + 7, SPEED_DOWN_Y + 5, depth, COLOR_LGRAY,
			1, TEXT_LEFT);
}

#include <citro2d.h>
#include "button.h"
#include "text.h"
#include "border.h"
#include "../../rendering/colors.h"
#include "../../audio/music.h"
#include "../../util/dispatcher.h"
#include "../../util/touchinput.h"

#define BOX_X		(MUSIC_LEFT_X - 10)
#define BOX_Y		55
#define BOX_WIDTH	120
#define BOX_HEIGHT	(EXIT_Y - BOX_Y + 40)
#define MUSIC_LEFT_X	(160 - 48 - 2)            /* Button width is 48 */
#define MUSIC_LEFT_Y	(BOX_Y + 10)
#define MUSIC_RIGHT_X	(160 + 2)
#define MUSIC_RIGHT_Y	(BOX_Y + 10)
#define RESTART_X	MUSIC_LEFT_X
#define RESTART_Y	(MUSIC_LEFT_Y + 30 + 15)  /* Button height is 30 */
#define EXIT_X		MUSIC_LEFT_X
#define EXIT_Y		(RESTART_Y + 30 + 5)      /* Button height is 30 */

static Button leftButton, rightButton, restartButton, exitButton;
static Text   leftText,   rightText,   restartText,   exitText;

static bool isShowing;
static Music_Song *curSong, exitSong;

// Declarations needed for buttons, dispatcher
static void changeMusic(int change);
static void restartMusic();
static void hideSelector();
static bool handleTouchInput();

bool SongSelector_Init() {
	leftButton = Button_Create(MUSIC_LEFT_X, MUSIC_LEFT_Y,
			SPRITE_SMALL_BUTTON, -1,
			(void*)-1, (void(*)(void*))changeMusic);
	if (!leftButton) goto f_leftButton;
	Button_Disable(leftButton);

	rightButton = Button_Create(MUSIC_RIGHT_X, MUSIC_RIGHT_Y,
			SPRITE_SMALL_BUTTON, -1,
			(void*)1, (void(*)(void*))changeMusic);
	if (!rightButton) goto f_rightButton;
	Button_Disable(rightButton);

	restartButton = Button_Create(RESTART_X, RESTART_Y, SPRITE_MEDIUM_BUTTON,
			-1, NULL, restartMusic);
	if (!restartButton) goto f_restartButton;
	Button_Disable(restartButton);

	exitButton = Button_Create(EXIT_X, EXIT_Y, SPRITE_MEDIUM_BUTTON,
			-1, NULL, hideSelector);
	if (!exitButton) goto f_exitButton;
	Button_Disable(exitButton);

	leftText = Text_Create(2);
	if (!leftText) goto f_leftText;
	Text_SetContent(leftText, "<");

	rightText = Text_Create(2);
	if (!rightText) goto f_rightText;
	Text_SetContent(rightText, ">");

	restartText = Text_Create(16);
	if (!restartText) goto f_restartText;
	Text_SetContent(restartText, "Restart");

	exitText = Text_Create(8);
	if (!exitText) goto f_exitText;
	Text_SetContent(exitText, "Done");

	isShowing = false;

	return true;

f_exitText:
	Text_Free(restartText);
f_restartText:
	Text_Free(rightText);
f_rightText:
	Text_Free(leftText);
f_leftText:
	Button_Free(exitButton);
f_exitButton:
	Button_Free(restartButton);
f_restartButton:
	Button_Free(rightButton);
f_rightButton:
	Button_Free(leftButton);
f_leftButton:
	return false;
}

void SongSelector_Exit() {
	Button_Free(exitButton);
	Button_Free(restartButton);
	Button_Free(rightButton);
	Button_Free(leftButton);
	Text_Free(exitText);
	Text_Free(restartText);
	Text_Free(rightText);
	Text_Free(leftText);
}

void SongSelector_Show(Music_Song *toEdit, Music_Song restore) {
	if (isShowing) return;

	isShowing = true;
	curSong = toEdit;
	exitSong = restore;
	Button_Enable(leftButton);
	Button_Enable(rightButton);
	Button_Enable(restartButton);
	Button_Enable(exitButton);
	Music_Start(*curSong);
}

static void hideSelector() {
	isShowing = false;
	Button_Disable(leftButton);
	Button_Disable(rightButton);
	Button_Disable(restartButton);
	Button_Disable(exitButton);
	Music_Start(exitSong);
}

static void changeMusic(int change) {
	*curSong = (*curSong + NUM_MUSIC_SONGS + change) % NUM_MUSIC_SONGS;
	Music_Start(*curSong);
}

static void restartMusic() {
	Music_Start(*curSong);
}

static bool handleTouchInput() {
	// If this is a popup, consume all touch events that weren't handled
	// by a button
	return (TouchInput_InProgress() || TouchInput_JustFinished()) && isShowing;
}

bool SongSelector_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority) {
	if (!Button_RegisterForTouchEvents(leftButton, touchDispatcher, priority))
		goto f_leftButton;
	if (!Button_RegisterForTouchEvents(rightButton, touchDispatcher, priority))
		goto f_rightButton;
	if (!Button_RegisterForTouchEvents(restartButton, touchDispatcher, priority))
		goto f_restartButton;
	if (!Button_RegisterForTouchEvents(exitButton, touchDispatcher, priority))
		goto f_exitButton;
	if (!Dispatcher_AddHandler(touchDispatcher, (Dispatcher_Handler) {
			.priority = priority, NULL, handleTouchInput }))
		goto f_handleTouchInput;

	return true;

f_handleTouchInput:
	Button_RemoveFromTouchDispatcher(exitButton, touchDispatcher);
f_exitButton:
	Button_RemoveFromTouchDispatcher(restartButton, touchDispatcher);
f_restartButton:
	Button_RemoveFromTouchDispatcher(rightButton, touchDispatcher);
f_rightButton:
	Button_RemoveFromTouchDispatcher(leftButton, touchDispatcher);
f_leftButton:
	return false;
}

void SongSelector_RemoveFromTouchDispatcher(Dispatcher touchDispatcher) {
	Button_RemoveFromTouchDispatcher(leftButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(rightButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(restartButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(exitButton, touchDispatcher);	Dispatcher_RemoveHandler(touchDispatcher, (Dispatcher_Handler) {
			.priority = 0, NULL, handleTouchInput });
}

void SongSelector_Draw(float depth) {
	if (!isShowing) return;

	float below = nextafter(depth, -1);
	C2D_DrawRectSolid(0, 0, below, 320, 240, COLOR_DGRAY & 0x77FFFFFF);

	Border_Draw(BOX_X, BOX_Y, below, BOX_WIDTH, BOX_HEIGHT);
	C2D_DrawRectSolid(BOX_X, BOX_Y, below, BOX_WIDTH, BOX_HEIGHT, COLOR_LGRAY);

	Button_Draw(leftButton, depth);
	Button_Draw(rightButton, depth);
	Button_Draw(restartButton, depth);
	Button_Draw(exitButton, depth);

	Text_Draw(leftText, MUSIC_LEFT_X + 16, MUSIC_LEFT_Y - 6, depth,
			COLOR_LGRAY, 2, TEXT_LEFT);
	Text_Draw(rightText, MUSIC_RIGHT_X + 18, MUSIC_RIGHT_Y - 6, depth,
			COLOR_LGRAY, 2, TEXT_LEFT);
	Text_Draw(restartText, RESTART_X + 10, RESTART_Y + 5, depth, COLOR_LGRAY,
			1, TEXT_LEFT);
	Text_Draw(exitText, EXIT_X + 10, EXIT_Y + 5, depth, COLOR_LGRAY, 1,
			TEXT_LEFT);
}

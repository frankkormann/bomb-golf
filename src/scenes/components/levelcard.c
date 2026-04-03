#include <stdbool.h>
#include <malloc.h>
#include "levelcard.h"
#include "button.h"
#include "text.h"
#include "../../scene.h"
#include "../editor.h"
#include "../course.h"
#include "../../rendering/colors.h"
#include "../../util/dispatcher.h"
#include "../../levelio.h"

#define TEXT_X 5
#define TEXT_Y 2
#define BUTTON_X 5
#define EDIT_BUTTON_Y 25
#define PLAY_BUTTON_Y 45

struct levelcard {
	int number;
	float x, y;
	Button mainButton;
	Button editButton;
	Button playButton;
	Text numberText;
};

static void editLevel(int levelNum) {
	Scene_SetNext(sceneEditor, Editor_MakeParams(levelNum));
}

static void playLevel(int levelNum) {
	Scene_SetNext(sceneCourse, Course_MakeParams(levelNum, false));
}

static void doNothing(void *ignored) {}

static bool checkLevelExists(int levelNum) {
	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(levelNum, false, path);
	FILE *level = fopen(path, "rb");
	bool exists = level != NULL;
	if (level) fclose(level);
	return exists;
}

LevelCard LevelCard_Create(float x, float y, int levelNum,
		void (*onPress)(int levelNum)) {
	LevelCard levelCard = malloc(sizeof(struct levelcard));
	if (!levelCard) goto f_levelCard;

	levelCard->mainButton = Button_Create(x, y, SPRITE_LEVEL_CARD_BACKGROUND,
			(void*)levelNum, (void (*)(void*))onPress);
	if (!levelCard->mainButton) goto f_mainButton;

	levelCard->editButton = Button_Create(x + BUTTON_X, y + EDIT_BUTTON_Y,
			SPRITE_LEVEL_EDIT_BUTTON, (void*)levelNum,
			(void (*)(void*))editLevel);
	if (!levelCard->editButton) goto f_editButton;

	bool levelExists = checkLevelExists(levelNum);
	levelCard->playButton = Button_Create(x + BUTTON_X, y + PLAY_BUTTON_Y,
			levelExists ? SPRITE_LEVEL_PLAY_BUTTON
			            : SPRITE_LEVEL_PLAY_BUTTON_DISABLED,
			(void*)levelNum,
			levelExists ? (void (*)(void*))playLevel : doNothing);
	if (!levelCard->playButton) goto f_playButton;

	levelCard->numberText = Text_Create(3);  // Two-digit numbers
	if (!levelCard->numberText) goto f_numberText;
	Text_SetContent(levelCard->numberText, "%i", levelNum);

	levelCard->x = x;
	levelCard->y = y;
	levelCard->number = levelNum;

	return levelCard;

f_numberText:
	Button_Free(levelCard->playButton);
f_playButton:
	Button_Free(levelCard->editButton);
f_editButton:
	Button_Free(levelCard->mainButton);
f_mainButton:
	free(levelCard);
f_levelCard:
	return NULL;
}

void LevelCard_Free(LevelCard levelCard) {
	Button_Free(levelCard->mainButton);
	Button_Free(levelCard->playButton);	
	Button_Free(levelCard->editButton);
}

bool LevelCard_RegisterForTouchEvents(LevelCard levelCard,
		Dispatcher touchDispatcher, int priority) {
	// Add these first to give them higher priority
	if (!Button_RegisterForTouchEvents(levelCard->editButton, touchDispatcher,
			priority)) goto f_editButton;
	if (!Button_RegisterForTouchEvents(levelCard->playButton, touchDispatcher,
			priority)) goto f_playButton;
	// Add this last for lowest priority
	if (!Button_RegisterForTouchEvents(levelCard->mainButton, touchDispatcher,
			priority)) goto f_mainButton;

	return true;

f_playButton:
	Button_RemoveFromTouchDispatcher(levelCard->editButton, touchDispatcher);
f_editButton:
	Button_RemoveFromTouchDispatcher(levelCard->mainButton, touchDispatcher);
f_mainButton:
	return false;
}

void LevelCard_RemoveFromTouchDispatcher(LevelCard levelCard,
		Dispatcher touchDispatcher) {
	Button_RemoveFromTouchDispatcher(levelCard->mainButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(levelCard->editButton, touchDispatcher);
	Button_RemoveFromTouchDispatcher(levelCard->playButton, touchDispatcher);
}

void LevelCard_Draw(LevelCard levelCard, float depth) {
	Button_Draw(levelCard->mainButton, depth);
	Button_Draw(levelCard->editButton, depth + 0.1);
	Button_Draw(levelCard->playButton, depth + 0.1);
	Text_Draw(levelCard->numberText, levelCard->x + TEXT_X,
			levelCard->y + TEXT_Y, depth + 0.1, COLOR_LGRAY, 1);
}

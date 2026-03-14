#include <malloc.h>
#include <citro2d.h>
#include "button.h"
#include "../../util/dispatcher.h"
#include "../../util/touchinput.h"
#include "../../rendering/spritesheet.h"

struct button {
	float x;
	float y;
	SpriteSheet_Sprite icon;
	void (*onTouch)(void);
};

Button Button_Create(float x, float y, SpriteSheet_Sprite icon,
		void (*onTouch)(void)) {
	Button button = malloc(sizeof(struct button));
	if (!button) return NULL;

	button->x = x;
	button->y = y;
	button->icon = icon;
	button->onTouch = onTouch;

	return button;
}

void Button_Free(Button button) {
	free(button);
}

static bool touchWithinBounds(Button button, touchPosition touch) {
	C2D_Image iconImg = SpriteSheet_GetImage(button->icon);
	return touch.px >= button->x
		&& touch.px < button->x + iconImg.subtex->width
		&& touch.py >= button->y
		&& touch.py < button->y + iconImg.subtex->height;
}

static bool handleTouch(void* buttonParam) {
	if (!TouchInput_InProgress() && !TouchInput_JustFinished()) return false;

	Button button = (Button)buttonParam;
	TouchInput_Swipe touch = TouchInput_GetSwipe();
	if (!touchWithinBounds(button, touch.start)) return false;

	if (touchWithinBounds(button, touch.end) && TouchInput_JustFinished()) {
		button->onTouch();
	}

	return true;
}

bool Button_RegisterForTouchEvents(Button button, Dispatcher touchDispatcher,
		int priority) {
	return Dispatcher_AddHandler(touchDispatcher,
			(Dispatcher_Handler) { priority, button, handleTouch });
}

void Button_RemoveFromTouchDispatcher(Button button, Dispatcher touchDispatcher) {
	Dispatcher_RemoveHandler(touchDispatcher,
			(Dispatcher_Handler) { 0, button, handleTouch });
}

void Button_Draw(Button button, float depth) {
	C2D_Image iconImg = SpriteSheet_GetImage(button->icon);
	SpriteSheet_Draw(
			button->icon,
			button->x + iconImg.subtex->width/2,
			button->y + iconImg.subtex->height/2,
			depth,
			0,
			false,
			false,
			NULL
		);
}

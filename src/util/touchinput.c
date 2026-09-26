#include <3ds.h>
#include "touchinput.h"

#define STICKY_LENIENCY 4

static touchPosition start, end, current;
static unsigned int counter;
static TouchInput_Mode flags;

// Handles TouchInput_Mode flags
static void readTouch(touchPosition *out) {
	static unsigned int framesWithoutInput;

	touchPosition touch;
	hidTouchRead(&touch);
	if (flags & TOUCHINPUT_STICKY && touch.px == 0 && touch.py == 0) {
		framesWithoutInput++;
		if (framesWithoutInput > STICKY_LENIENCY) {
			*out = touch;
		}
	} else {
		framesWithoutInput = 0;
		if (flags & TOUCHINPUT_MIRROR && (touch.px != 0 || touch.py != 0)) {
			touch.px = 320 - touch.px;
		}
		*out = touch;
	}
}

void TouchInput_Scan() {
	if (current.px == 0 && current.py == 0) {
		start = end = current;
		counter = 0;
	}

	readTouch(&current);
	if (current.px != 0 || current.py != 0) {
		if (counter == 0) {
			start = current;
		}
		end = current;
		counter++;
	}
}

bool TouchInput_JustStarted() {
	return counter == 1;
}

bool TouchInput_InProgress() {
	return counter > 0 && (current.px != 0 || current.py != 0);
}

bool TouchInput_JustFinished() {
	return counter > 0 && current.px == 0 && current.py == 0;
}

TouchInput_Swipe TouchInput_GetSwipe() {
	return (TouchInput_Swipe) {
		.start = start,
		.end = end,
		.length = counter
	};
}

void TouchInput_SetMode(TouchInput_Mode argFlags) {
	flags = argFlags;
}

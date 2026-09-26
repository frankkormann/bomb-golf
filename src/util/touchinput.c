#include <3ds.h>
#include "touchinput.h"

static touchPosition start, end, current;
static unsigned int counter;
static TouchInput_Mode flags;

void TouchInput_Scan() {
	if (current.px == 0 && current.py == 0) {
		start = end = current;
		counter = 0;
	}

	hidTouchRead(&current);
	if (flags & TOUCHINPUT_MIRROR && (current.px != 0 || current.py != 0)) {
		current.px = 320 - current.px;
	}

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

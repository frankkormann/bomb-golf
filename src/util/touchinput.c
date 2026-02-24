#include <3ds.h>
#include "touchinput.h"

static touchPosition start, end, current;
static unsigned int counter;

void TouchInput_Scan() {
	if (current.px == 0 && current.py == 0) {
		start = end = current;
		counter = 0;
	}

	hidTouchRead(&current);

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

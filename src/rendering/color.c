#include "color.h"

u32 Color_ForScore(int strokes, int par) {
	//TODO Decide on better colors?
	if (strokes == 1) return COLOR_WHITE;

	int score = strokes - par;
	if (score <= -2) {
		return COLOR_YELLOW;
	} else if (score <= -1) {
		return COLOR_BLUE;
	} else if (score <= 0) {
		return COLOR_GREEN;
	} else {
		return COLOR_DBROWN;
	}
}

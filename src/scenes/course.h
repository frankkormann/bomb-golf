/*
 * Scene where the player plays through a level.
 */

#ifndef COURSE_H
#define COURSE_H

#include <stdbool.h>
#include "../scene.h"

typedef struct {
	int level;
	bool inRomfs;
} Course_Params;

extern Scene sceneCourse;

int Course_GetFieldWidth();
int Course_GetFieldHeight();

/*
 * Returns the number of pixels the bottom screen has been shifted horizontally
 * before being drawn. Useful for converting touchscreen coordinates to in-game
 * coordinates.
 */
int Course_GetScreenOffset();

#endif

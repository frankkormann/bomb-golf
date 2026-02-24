/*
 * Scene where the player plays through a level.
 */

#ifndef COURSE_H
#define COURSE_H

#include <stdbool.h>

typedef struct {
	unsigned int level;
	bool isSdmc;
} Course_Params;

#include "../scene.h"

extern Scene sceneCourse;

/*
 * Makes Scene_Params to load the course with number level. If isSdmc is true,
 * looks for the file in the SD card.
 */
Scene_Params Course_MakeParams(unsigned int level, bool isSdmc);

/*
 * Clears a circle of terrain of radius centered at (x, y).
 */
void Course_ClearCircle(int x, int y, int radius);

/*
 * Returns true if there is terrain at (x, y).
 */
bool Course_CheckTerrain(int x, int y);

int Course_GetFieldWidth();
int Course_GetFieldHeight();

#endif

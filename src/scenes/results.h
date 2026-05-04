/*
 * Scene to display results after completing a course.
 */

#ifndef RESULTS_H
#define RESULTS_H

typedef struct {
	int strokes;
	int level;
	bool levelInRomfs;
} Results_Params;

#include "../scene.h"

extern Scene sceneResults;

/*
 * Makes Scene_Params to show the specified level having been completed in
 * strokes strokes.
 */
Scene_Params Results_MakeParams(int strokes, int level, bool levelInRomfs);

#endif

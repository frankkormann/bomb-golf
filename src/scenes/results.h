/*
 * Scene to display results after completing a course.
 *
 * Note: This scene requires the terrain to still be loaded when it is
 * initialized.
 */

#ifndef RESULTS_H
#define RESULTS_H

#include "components/background.h"
#include "components/tracer.h"

typedef struct {
	int strokes;
	int level;
	bool levelInRomfs;
	Tracer projPath;
} Results_Params;

#include "../scene.h"

extern Scene sceneResults;

/*
 * Makes Scene_Params to show the specified level having been completed in
 * strokes strokes.
 */
Scene_Params Results_MakeParams(int strokes, int level, bool levelInRomfs,
		Tracer projPath);

#endif

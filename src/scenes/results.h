/*
 * Scene to display results after completing a course.
 *
 * Note: This scene requires the terrain to still be loaded when it is
 * initialized.
 */

#ifndef RESULTS_H
#define RESULTS_H

#include "../scene.h"
#include "components/background.h"
#include "components/tracer.h"

typedef struct {
	int strokes;
	int level;
	bool levelInRomfs;
	bool isSequence;
	Tracer projPath;
} Results_Params;

extern Scene sceneResults;

#endif

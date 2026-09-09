/*
 * Scene for creating and editing level files.
 */

#ifndef EDITOR_H
#define EDITOR_H

// Not including null terminator
#define EDITOR_LEVEL_NAME_MAX 24

#include "../scene.h"

typedef struct {
	int level;
} Editor_Params;

extern Scene sceneEditor;

#endif

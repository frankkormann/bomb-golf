/*
 * Scene for creating and editing level files.
 */

#ifndef EDITOR_H
#define EDITOR_H

typedef struct {
	unsigned int level;
} Editor_Params;

#include "../scene.h"

extern Scene sceneEditor;

/*
 * Makes Scene_Params to create or edit the course with number level.
 */
Scene_Params Editor_MakeParams(unsigned int level);

#endif

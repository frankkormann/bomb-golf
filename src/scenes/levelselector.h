/*
 * Scene for selecting a custom level to play/edit.
 */

#ifndef LEVELSELECTOR_H
#define LEVELSELECTOR_H

typedef struct {
	int level;
} LevelSelector_Params;

#include "../scene.h"

extern Scene sceneLevelSelector;

/*
 * Makes Scene_Params so that the level with number level is previewed by
 * default. If level is -1, no level is previewed.
 */
Scene_Params LevelSelector_MakeParams(int level);

#endif

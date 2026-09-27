/*
 * Scene for selecting a custom level to play/edit.
 */

#ifndef LEVELSELECTOR_H
#define LEVELSELECTOR_H

#include <stdbool.h>
#include "../scene.h"

typedef struct {
	int level;
	bool inRomfs;
} LevelSelector_Params;

extern Scene sceneLevelSelector;

#endif

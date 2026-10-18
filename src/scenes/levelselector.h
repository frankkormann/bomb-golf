/*
 * Scene for selecting a custom level to play/edit.
 */

#ifndef LEVELSELECTOR_H
#define LEVELSELECTOR_H

#include <stdbool.h>
#include "../scene.h"
#include "../projectile.h"

typedef struct {
	int level;
	bool inRomfs;
	/* Give NULL to continue using the last-selected projectile */
	Projectile selectedProj;
} LevelSelector_Params;

extern Scene sceneLevelSelector;

#endif

/*
 * Scene for selecting a custom level to play/edit.
 */

#ifndef LEVEL_SELECTOR_H
#define LEVEL_SELECTOR_H

typedef struct {} LevelSelector_Params;

#include "../scene.h"

extern Scene sceneLevelSelector;

/*
 * Makes empty Scene_Params.
 */
Scene_Params LevelSelector_MakeParams();

#endif

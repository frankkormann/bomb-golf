#ifndef SCENE_INTERNAL_H
#define SCENE_INTERNAL_H

#include "../scene.h"

struct scene {
	bool (*const init)(Scene_Params params);
	void (*const update)(void);
	void (*const draw)(void);
	void (*const exit)(void);
};

#endif

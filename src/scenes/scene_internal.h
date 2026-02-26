/*
 * Struct which should be completed by any implementors of a Scene.
 */

#ifndef SCENE_INTERNAL_H
#define SCENE_INTERNAL_H

#include "../scene.h"

struct scene {
	/*
	 * Initializes the Scene.
	 *
	 * Returns false if an error occurred and the Scene could not be intialized.
	 */
	bool (*const init)(Scene_Params params);
	/*
	 * Function which should be called once per physics frame.
	 */
	void (*const update)(void);
	/*
	 * Function which should be called once per graphics frame.
	 */
	void (*const draw)(void);
	/*
	 * Exits and deallocates any resources used by the Scene.
	 */
	void (*const exit)(void);
};

#endif

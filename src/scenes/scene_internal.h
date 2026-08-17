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
	 * Function which should be called once per frame to update its state.
	 */
	void (*const update)(float speed);
	/*
	 * Function which draws everything to the screen; should not update scene
	 * state.
	 *
	 * Must call Animation_Draw somewhere with appropriate depth.
	 */
	void (*const draw)(void);
	/*
	 * Exits and deallocates any resources used by the Scene.
	 */
	void (*const exit)(void);
};

#endif

/*
 * Manages switching between different Scenes. Also hooks into the Animation
 * module and Popup component, making these available to each Scene.
 */

#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

// GCC doesn't warn for missing field initializers if a struct is
// empty-intialized, like struct foo {}. But we always want these warnings in
// case a Scene gains parameters. This hack gets around that somewhat.
typedef struct {
	char _;
} Scene_EmptyParams;
#define SCENE_PARAMS_EMPTY	{{ 1 }}
#define SCENE_PARAMS_EMPTY_DEF	Scene_EmptyParams _;

typedef struct scene *Scene;

/*
 * Sets first as the active Scene. Each implementor of Scene provides a way
 * to make their params, usually a struct to fill int.
 *
 * Resets speed to 1 (see Scene_SetSpeed).
 *
 * Returns false if the Scene was unable to intialized. (Note that a different
 * Scene may have been initialized instead to display the error.)
 */
bool Scene_Start(Scene first, void *params);

/*
 * Updates the active Scene.
 */
void Scene_Update();

/*
 * Draws the active Scene.
 */
void Scene_Draw();

/*
 * Exits the active Scene without setting a new active Scene.
 */
void Scene_Exit();

/*
 * Exits the active Scene and sets next as the new active Scene. Each
 * implementor of Scene provides a way to make their params, usually a struct
 * to fill int.
 *
 * If this is called from inside a Scene, it does not return; the new Scene
 * is switched to immediately.
 *
 * Also calls Animation_Clear and resets speed to 1 (see Scene_SetSpeed).
 */
void Scene_Switch(Scene next, void *params);

/*
 * Affects how much the active Scene updates for each call to Scene_Update.
 * The speed acts as a multiplier; 1 is typical, 0.5 is half as fast, 2 is
 * twice as fast, etc. Each Scene may interpret this number as it wishes.
 *
 * This is reset to 1 when a new Scene is entered.
 */
void Scene_SetSpeed(float speed);

#endif

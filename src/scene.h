/*
 * Manage switching between different Scenes.
 */

#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>

typedef struct scene *Scene;
// Make Scene_Params visible to each scene header
typedef union scene_params Scene_Params;

#include "scenes/course.h"
#include "scenes/editor.h"
#include "scenes/title.h"

union scene_params {
	Course_Params course;
	Editor_Params editor;
	Title_Params title;
};

/*
 * Sets first as the active Scene.
 * Each implementor of Scene provides a function to make their Scene_Params.
 *
 * Returns false if the Scene was unable to intialized.
 */
bool Scene_Start(Scene first, Scene_Params params);

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
 * Exits the active Scene and sets next as the new active Scene.
 * Each implementor of Scene provides a function to make their Scene_Params.
 */
void Scene_SetNext(Scene next, Scene_Params params);

#endif

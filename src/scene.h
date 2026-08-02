/*
 * Manages switching between different Scenes. Also hooks into the Animation
 * module and Popup component, making these available to each Scene.
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
#include "scenes/error.h"
#include "scenes/levelselector.h"
#include "scenes/results.h"

union scene_params {
	Course_Params course;
	Editor_Params editor;
	Title_Params title;
	Error_Params error;
	LevelSelector_Params levelselector;
	Results_Params results;
};

/*
 * Sets first as the active Scene.
 * Each implementor of Scene provides a function to make their Scene_Params.
 * Resets speed to 1 (see Scene_SetSpeed).
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
 *
 * Also calls Animation_Clear and resets speed to 1 (see Scene_SetSpeed).
 */
void Scene_SetNext(Scene next, Scene_Params params);

/*
 * Affects how much the active Scene updates for each call to Scene_Update.
 * The speed acts as a multiplier; 1 is typical, 0.5 is half as fast, 2 is
 * twice as fast, etc. Each Scene may interpret this number as it wishes.
 *
 * This is reset to 1 when a new Scene is entered.
 */
void Scene_SetSpeed(float speed);

#endif

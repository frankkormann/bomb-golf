#include <stddef.h>
#include "scene.h"
#include "scenes/scene_internal.h"

static Scene sceneCurrent;
static Scene sceneNext;
static Scene_Params nextParams;

bool Scene_Start(Scene first, Scene_Params params) {
	bool success = first->init(params);
	sceneCurrent = success ? first : NULL;
	return success;
}

void Scene_Update() {
	if (sceneNext) {
		Scene_Exit();
		Scene_Start(sceneNext, nextParams);
		sceneNext = NULL;
	}
	if (sceneCurrent) sceneCurrent->update();
}

void Scene_Draw() {
	if (sceneCurrent) sceneCurrent->draw();
}

void Scene_Exit() {
	if (sceneCurrent) sceneCurrent->exit();
	sceneCurrent = NULL;
}

void Scene_SetNext(Scene next, Scene_Params params) {
	sceneNext = next;
	nextParams = params;
}

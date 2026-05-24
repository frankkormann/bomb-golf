#include <stddef.h>
#include "scene.h"
#include "scenes/scene_internal.h"
#include "scenes/components/popup.h"
#include "rendering/animation.h"

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
		// Do this shuffling in case Scene_Start calls Scene_SetNext
		Scene toStart = sceneNext;
		sceneNext = NULL;
		Scene_Exit();
		Animation_Clear(false);
		Scene_Start(toStart, nextParams);
	}
	if (sceneCurrent) {
		if (!Popup_IsOpen()) {
			sceneCurrent->update();
			Animation_Update();
		} else {
			Popup_Update();
		}
	}
}

void Scene_Draw() {
	if (sceneCurrent) {
		sceneCurrent->draw();
		Animation_Draw();
		if (Popup_IsOpen()) Popup_Draw();
	}
}

void Scene_Exit() {
	if (sceneCurrent) sceneCurrent->exit();
	sceneCurrent = NULL;
}

void Scene_SetNext(Scene next, Scene_Params params) {
	sceneNext = next;
	nextParams = params;
}

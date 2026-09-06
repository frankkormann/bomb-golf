#include <stddef.h>
#include <setjmp.h>
#include "scene.h"
#include "scenes/scene_internal.h"
#include "scenes/components/popup.h"
#include "rendering/animation.h"

static Scene scene;
static bool inScene;
static jmp_buf jmpbuf;
static float speed;

bool Scene_Start(Scene first, void *params) {
	bool success = first->init(params);
	scene = success ? first : NULL;
	speed = 1;
	return success;
}

void Scene_Update() {
	inScene = true;
	if (setjmp(jmpbuf) != 0) return;
	if (scene) {
		if (!Popup_IsOpen()) {
			scene->update(speed);
			Animation_Update(speed);
		} else {
			Popup_Update();
		}
	}
	inScene = false;
}

void Scene_Draw() {
	if (scene) {
		scene->draw();
		if (Popup_IsOpen()) Popup_Draw();
	}
}

void Scene_Exit() {
	if (scene) scene->exit();
	scene = NULL;
}

void Scene_Switch(Scene next, void *params) {
	Scene_Exit();
	Animation_Clear(false);
	Scene_Start(next, params);
	if (inScene) {
		longjmp(jmpbuf, 1);
	}
}

void Scene_SetSpeed(float argSpeed) {
	speed = argSpeed;
}

#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stdlib.h>
#include "savedata.h"
#include "scene.h"
#include "scenes/title.h"
#include "scenes/error.h"
#include "scenes/components/text.h"
#include "scenes/components/popup.h"
#include "rendering/spritesheet.h"
#include "rendering/rendertarget.h"
#include "rendering/animation.h"
#include "util/touchinput.h"

static bool pausePopupVisible;

static void resumeGame() {
	if (!pausePopupVisible) return;
	Popup_Exit();
	pausePopupVisible = false;
}

int main() {
	romfsInit();
	gfxInitDefault();

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C3D_FrameRate(60);
	C2D_Prepare();
	
	SpriteSheet_Init();
	Text_Init();
	Scene_Start(sceneTitle, Title_MakeParams());

	if (!SaveData_Mount()) {
		Scene_SetNext(sceneError,
				Error_MakeParams("Failed to mount save data"));
	}

	pausePopupVisible = false;

	while (aptMainLoop()) {
		hidScanInput();
		TouchInput_Scan();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) {
			Popup_Button buttons[1] = {
					{ "Resume", NULL, resumeGame }
				};
			if (Popup_Init("Paused", ONE_BUTTON, buttons)) {
				pausePopupVisible = true;
			}			
		}

		if (!pausePopupVisible) {
			Scene_Update();
			Animation_Update();
		} else {
			Popup_Update();
		}

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		Scene_Draw();
		Animation_Draw();
		if (pausePopupVisible) Popup_Draw();
		C3D_FrameEnd(0);
	}

	SaveData_Unmount();
	Scene_Exit();
	Text_Exit();
	SpriteSheet_Exit();
	RenderTarget_DeleteAll();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}

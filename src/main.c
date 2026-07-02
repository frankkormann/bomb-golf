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

int main() {
	romfsInit();
	gfxInitDefault();
	ndspInit();
	ndspSetOutputMode(NDSP_OUTPUT_STEREO);

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C3D_FrameRate(60);
	C2D_Prepare();
	
	SpriteSheet_Init();
	Text_Init();
	Animation_Init();
	Scene_Start(sceneTitle, Title_MakeParams());

	if (!SaveData_Mount()) {
		Scene_SetNext(sceneError,
				Error_MakeParams("Failed to mount save data"));
	}

	while (aptMainLoop()) {
		hidScanInput();
		TouchInput_Scan();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) {
			Popup_Button buttons[] = {
					{ "Resume", KEY_START, NULL, Popup_Exit }
				};
			Popup_Init("Paused", POPUP_ONE_BUTTON, buttons);
		}
		#ifndef _CIA
			if (kDown & KEY_SELECT) break;
		#endif

		Scene_Update();

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		Scene_Draw();
		C3D_FrameEnd(0);
	}

	SaveData_Unmount();
	Scene_Exit();
	Animation_Exit();
	Text_Exit();
	SpriteSheet_Exit();
	RenderTarget_DeleteAll();
	C2D_Fini();
	C3D_Fini();
	ndspExit();
	gfxExit();
	romfsExit();
	return 0;
}

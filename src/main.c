#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stdlib.h>
#include "scene.h"
#include "scenes/title.h"
#include "rendering/spritesheet.h"
#include "rendering/rendertarget.h"
#include "util/touchinput.h"

int main() {
	romfsInit();
	gfxInitDefault();

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	
	SpriteSheet_Init();
	Scene_Start(sceneTitle, Title_MakeParams());

	while (aptMainLoop()) {
		hidScanInput();
		TouchInput_Scan();
		
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) break;

		Scene_Update();

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		Scene_Draw();
		C3D_FrameEnd(0);
	}

	Scene_Exit();
	SpriteSheet_Exit();
	RenderTarget_DeleteAll();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}

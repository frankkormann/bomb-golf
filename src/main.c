#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stdlib.h>
#include "scene.h"
#include "scenes/title.h"
#include "rendering/spritesheet.h"
#include "rendering/rendertarget.h"
#include "rendering/animation.h"
#include "util/touchinput.h"

int main() {
	romfsInit();
	gfxInitDefault();

	C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
	C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
	C2D_Prepare();
	
	SpriteSheet_Init();
	Scene_Start(sceneTitle, Title_MakeParams());

#ifdef _CIA
	Result res = archiveMount(ARCHIVE_SAVEDATA, fsMakePath(PATH_EMPTY, ""),
			"save");
	if (R_FAILED(res)) {
		res = FSUSER_FormatSaveData(ARCHIVE_SAVEDATA,
				fsMakePath(PATH_EMPTY, ""),
				0x200, 0, 3, 3, 3, false);
		if (R_FAILED(res)) return 1;
		res = archiveMount(ARCHIVE_SAVEDATA, fsMakePath(PATH_EMPTY, ""),
				"save");
		if (R_FAILED(res)) return 1;
		//TODO Have an error scene
	}
#endif

	while (aptMainLoop()) {
		hidScanInput();
		TouchInput_Scan();
		
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START) break;

		Scene_Update();
		Animation_Update();

		C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
		Scene_Draw();
		Animation_Draw();
		C3D_FrameEnd(0);
	}

#ifdef _CIA
	archiveCommitSaveData("save");
	archiveUnmount("save");
#endif

	Scene_Exit();
	SpriteSheet_Exit();
	RenderTarget_DeleteAll();
	C2D_Fini();
	C3D_Fini();
	gfxExit();
	romfsExit();
	return 0;
}

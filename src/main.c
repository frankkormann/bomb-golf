#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stdlib.h>
#include "main.h"
#include "scene.h"
#include "scenes/title.h"
#include "rendering/spritesheet.h"
#include "rendering/rendertarget.h"
#include "rendering/animation.h"
#include "util/touchinput.h"

#ifdef _CIA
//https://www.3dbrew.org/wiki/RomFS#Hash_Table_Structure
static int getHashTableLength(int numEntries) {
	int count = numEntries;
	if (numEntries < 3) {
		count = 3;
	 } else if (numEntries < 19) {
		count |= 1;
	 } else {
		while (count % 2 == 0
				|| count % 3 == 0
				|| count % 5 == 0
				|| count % 7 == 0
				|| count % 11 == 0
				|| count % 13 == 0
				|| count % 17 == 0) {
			count++;
		}
	}
	return count;
}
#endif

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
		res = FSUSER_FormatSaveData(
				ARCHIVE_SAVEDATA,
				fsMakePath(PATH_EMPTY, ""),
				512,
				0,
				MAX_LEVEL_NUM - MIN_LEVEL_NUM + 1,
				getHashTableLength(0),
				getHashTableLength(
						MAX_LEVEL_NUM - MIN_LEVEL_NUM + 1),
				false
			);
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

#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "editor.h"
#include "levelselector.h"
#include "error.h"
#include "components/text.h"
#include "components/tileselector.h"
#include "components/background.h"
#include "components/border.h"
#include "components/button.h"
#include "../rendering/rendertarget.h"
#include "../rendering/colors.h"
#include "../rendering/spritesheet.h"
#include "../projectiles/ball.h"
#include "../util/touchinput.h"
#include "../util/macros.h"
#include "../levelio.h"

#define HOLE_WIDTH (TILE_SIZE * 2)
#define HOLE_HEIGHT (TILE_SIZE * 4)

#define SCROLL_UNIT TILE_SIZE

#define TEXT_MARGIN 10
#define LEVEL_NAME_Y 15
#define LEVEL_PREVIEW_X 10
#define LEVEL_PREVIEW_Y (LEVEL_NAME_Y + 35) 
#define LEVEL_PREVIEW_WIDTH 380
#define LEVEL_PREVIEW_HEIGHT 90
#define CONTROLS_TEXT_Y (LEVEL_PREVIEW_Y + LEVEL_PREVIEW_HEIGHT + 15)
#define RIGHT_MENU_X 200
#define RIGHT_MENU_Y (TILE_SELECTOR_HEIGHT + BORDER_WIDTH + 10)
#define RIGHT_MENU_WIDTH 120
#define RIGHT_MENU_HEIGHT (240 - RIGHT_MENU_Y - BORDER_WIDTH - 10)
#define RIGHT_MENU_BUTTON_GAP 35
#define RIGHT_MENU_BUTTON_X (RIGHT_MENU_X + 10)
#define RIGHT_MENU_BUTTON_Y (RIGHT_MENU_Y + 10)

static Background bg;
static float scroll;

static Tile (*tiles)[LEVEL_HEIGHT_TILES];

static int holeX, holeY;
static int projX, projY;
static int par;
static unsigned int level;
static char *name;

static enum { PENCIL, RECTANGLE } brushType;
static Button showDetailsButton, hideDetailsButton;

static Text nameText, parText, controlsText1, controlsText2;
static bool rightMenuOpen;
static Button editNameButton, saveButton, exitButton, parUpButton, parDownButton;
static Text   editNameText,   saveText,   exitText,   parUpText,   parDownText;

static Dispatcher touchDispatcher;

Scene_Params Editor_MakeParams(unsigned int level) {
	return (Scene_Params) { .editor = {
		.level = level
	} };
}

// Declarations needed to register with dispatcher, buttons
static bool handleTouchInput(void *ignored);
static bool handleTouchInputRightMenu(void *ignored);
static void toggleRightMenu(void *ignored);
static void editName(void *ignored);
static void saveExit(void *ignored);
static void exitNoSave(void *ignored);
static void changePar(int change);

static bool sceneInit(Scene_Params params) {
	char *errMsg = "";

	bg = BG_Create(LEVEL_MAX_WIDTH, LEVEL_HEIGHT, COLOR_BLUE);
	if (!bg) {
		errMsg = "Out of memory";
		goto f_bg;
	}

	nameText = Text_Create(EDITOR_LEVEL_NAME_MAX + 1);
	if (!nameText) {
		errMsg = "Out of memory";
		goto f_nameText;
	}

	parText = Text_Create(9);
	if (!parText) {
		errMsg = "Out of memory";
		goto f_parText;
	}

	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(params.editor.level, false, path);
	LevelIO_Hole hole;
	LevelIO_Proj proj;
	int width;
	if (LevelIO_Read(path, &hole, &proj, &tiles, &width, &par, &name)) {
		Tile (*newTiles)[LEVEL_HEIGHT_TILES] = realloc(tiles,
				sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!newTiles) {
			errMsg = "Out of memory";
			goto f_newTiles;
		}
		tiles = newTiles;

		for (int x = width / TILE_SIZE; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				tiles[x][y] = Tile_Make(SPRITE_TILE_SKY, 0);
			}
		}

		holeX = hole.x;
		holeY = hole.y;
		projX = proj.startX;
		projY = proj.startY;

		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				BG_DrawTile(bg, tiles[x][y], x * TILE_SIZE,
						y * TILE_SIZE, false);
			}
		}
	} else {
		tiles = malloc(sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!tiles) {
			errMsg = "Out of memory";
			goto f_tiles;
		}

		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				tiles[x][y] = Tile_Make(SPRITE_TILE_SKY, 0);
			}
		}

		holeX = holeY = 0;
		projX = 40 + (TILE_SIZE / 2);
		projY = 190 + (TILE_SIZE / 2);
		name = malloc(sizeof('\0'));
		name[0] = '\0';
	}
	Text_SetContent(nameText, name);

	controlsText1 = Text_Create(128);
	if (!controlsText1) {
		errMsg = "Out of memory";
		goto f_controlsText1;
	}
	controlsText2 = Text_Create(128);
	if (!controlsText2) {
		errMsg = "Out of memory";
		goto f_controlsText2;
	}

	Text_SetContent(controlsText1,
			"%c/%c: Switch brush",
			TEXT_KEY_L, TEXT_KEY_R
		);
	Text_SetContent(controlsText2,
			"%c (hold) + touchscreen: Move ball\n"
			"%c (hold) + touchscreen: Move hole",
			TEXT_KEY_DUP, TEXT_KEY_DDOWN
		);

	if (!TileSelector_Init(Tile_Make(SPRITE_TILE_SKY, 0))) {
		errMsg = "Out of memory";
		goto f_TileSelector;
	}

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) {
		errMsg = "Out of memory";
		goto f_touchDispatcher;
	}
	Dispatcher_AddHandler(touchDispatcher, (Dispatcher_Handler) {
			.priority = 0, NULL, handleTouchInput });
	TileSelector_RegisterForTouchEvents(touchDispatcher, 1);

	showDetailsButton = Button_Create(308, RIGHT_MENU_Y,
			SPRITE_BUTTON_LEFT, NULL, toggleRightMenu);
	if (!showDetailsButton) {
		errMsg = "Out of memory";
		goto f_showDetailsButton;
	}
	Button_RegisterForTouchEvents(showDetailsButton, touchDispatcher, 3);

	hideDetailsButton = Button_Create(RIGHT_MENU_X - BORDER_WIDTH - 11,
			RIGHT_MENU_Y, SPRITE_BUTTON_RIGHT, NULL,
			toggleRightMenu);
	if (!hideDetailsButton) {
		errMsg = "Out of memory";
		goto f_hideDetailsButton;
	}
	Button_RegisterForTouchEvents(hideDetailsButton, touchDispatcher, 3);
	Button_Disable(hideDetailsButton);

	editNameButton = Button_Create(RIGHT_MENU_BUTTON_X, RIGHT_MENU_BUTTON_Y,
			SPRITE_MEDIUM_BUTTON, NULL, editName);
	if (!editNameButton) {
		errMsg = "Out of memory";
		goto f_editNameButton;
	}
	Button_RegisterForTouchEvents(editNameButton, touchDispatcher, 3);
	Button_Disable(editNameButton);

	editNameText = Text_Create(16);
	if (!editNameText) {
		errMsg = "Out of memory";
		goto f_editNameText;
	}
	Text_SetContent(editNameText, "Edit Name");

	saveButton = Button_Create(RIGHT_MENU_BUTTON_X,
			RIGHT_MENU_BUTTON_Y + RIGHT_MENU_BUTTON_GAP,
			SPRITE_MEDIUM_BUTTON, NULL, saveExit);
	if (!saveButton) {
		errMsg = "Out of memory";
		goto f_saveButton;
	}
	Button_RegisterForTouchEvents(saveButton, touchDispatcher, 3);
	Button_Disable(saveButton);

	saveText = Text_Create(16);
	if (!saveText) {
		errMsg = "Out of memory";
		goto f_saveText;
	}
	Text_SetContent(saveText, "Save & Exit");

	exitButton = Button_Create(RIGHT_MENU_BUTTON_X,
			RIGHT_MENU_BUTTON_Y + 2*RIGHT_MENU_BUTTON_GAP,
			SPRITE_MEDIUM_BUTTON, NULL, exitNoSave);
	if (!exitButton) {
		errMsg = "Out of memory";
		goto f_exitButton;
	}
	Button_RegisterForTouchEvents(exitButton, touchDispatcher, 3);
	Button_Disable(exitButton);

	exitText = Text_Create(16);
	if (!exitText) {
		errMsg = "Out of memory";
		goto f_exitText;
	}
	Text_SetContent(exitText, "Exit");

	parUpButton = Button_Create(RIGHT_MENU_X + RIGHT_MENU_WIDTH - 58,
			RIGHT_MENU_BUTTON_Y + 3*RIGHT_MENU_BUTTON_GAP + 15,
			SPRITE_SMALL_BUTTON, (void*)1, (void(*)(void*))changePar);
	if (!parUpButton) {
		errMsg = "Out of memory";
		goto f_parUpButton;
	}
	Button_RegisterForTouchEvents(parUpButton, touchDispatcher, 3);
	Button_Disable(parUpButton);

	parUpText = Text_Create(2);
	if (!parUpText) {
		errMsg = "Out of memory";
		goto f_parUpText;
	}
	Text_SetContent(parUpText, "+");

	parDownButton = Button_Create(RIGHT_MENU_BUTTON_X,
			RIGHT_MENU_BUTTON_Y + 3*RIGHT_MENU_BUTTON_GAP + 15,
			SPRITE_SMALL_BUTTON, (void*)-1, (void(*)(void*))changePar);
	if (!parDownButton) {
		errMsg = "Out of memory";
		goto f_parDownButton;
	}
	Button_RegisterForTouchEvents(parDownButton, touchDispatcher, 3);
	Button_Disable(parDownButton);

	parDownText = Text_Create(2);
	if (!parDownText) {
		errMsg = "Out of memory";
		goto f_parDownText;
	}
	Text_SetContent(parDownText, "-");

	Dispatcher_AddHandler(touchDispatcher, (Dispatcher_Handler) {
			.priority = 2, NULL, handleTouchInputRightMenu });

	scroll = 0;
	level = params.editor.level;
	brushType = PENCIL;
	rightMenuOpen = false;

	return true;

f_parDownText:
	Button_Free(parDownButton);
f_parDownButton:
	Text_Free(parUpText);
f_parUpText:
	Button_Free(parUpButton);
f_parUpButton:
	Text_Free(exitText);
f_exitText:
	Button_Free(exitButton);
f_exitButton:
	Text_Free(saveText);
f_saveText:
	Button_Free(saveButton);
f_saveButton:
	Text_Free(editNameText);
f_editNameText:
	Button_Free(editNameButton);
f_editNameButton:
	Button_Free(hideDetailsButton);
f_hideDetailsButton:
	Button_Free(showDetailsButton);
f_showDetailsButton:
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	TileSelector_Exit();
f_TileSelector:
	Text_Free(controlsText2);
f_controlsText2:
	Text_Free(controlsText1);
f_controlsText1:
f_newTiles:
	free(tiles);
f_tiles:
	Text_Free(parText);
f_parText:
	Text_Free(nameText);
f_nameText:
	BG_Free(bg);
f_bg:
	Scene_SetNext(sceneError, Error_MakeParams(errMsg));
	return false;
}

static void sceneExit() {
	BG_Free(bg);
	free(tiles);
	free(name);
	Text_Free(controlsText1);
	Text_Free(controlsText2);
	Text_Free(nameText);
	Text_Free(parText);
	Dispatcher_Free(touchDispatcher);
	TileSelector_Exit();
	Button_Free(showDetailsButton);
	Button_Free(hideDetailsButton);
	Button_Free(editNameButton);
	Button_Free(saveButton);
	Button_Free(exitButton);
	Text_Free(editNameText);
	Text_Free(saveText);
	Text_Free(exitText);
	Button_Free(parUpButton);
	Button_Free(parDownButton);
	Text_Free(parUpText);
	Text_Free(parDownText);
}

static bool exportLevel() {
	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(level, false, path);

	LevelIO_Hole hole = { holeX, holeY, HOLE_WIDTH, HOLE_HEIGHT };
	LevelIO_Proj proj = { projX, projY, projectileBall };

	int tilesMaxX = 0;
	for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
		for (int x = tilesMaxX; x < LEVEL_MAX_WIDTH_TILES; x++) {
			if (Tile_GetSprite(tiles[x][y]) != SPRITE_TILE_SKY) {
				tilesMaxX = x;
			}
		}
	}

	return LevelIO_Write(path, hole, proj, tiles, (tilesMaxX + 1) * TILE_SIZE,
			par, name);
}

static void changeTile(int tileX, int tileY, Tile newTile) {
	tiles[tileX][tileY] = newTile;
	BG_DrawTile(bg, newTile, tileX * TILE_SIZE, tileY * TILE_SIZE, true);
}

// ignored param is to match the signature of Dispatcher_Handler
static bool handleTouchInput(void *ignored) {
	u32 kHeld = hidKeysHeld();

	if (TouchInput_InProgress()) {
		float courseX = TouchInput_GetSwipe().end.px + scroll;
		float courseY = TouchInput_GetSwipe().end.py;
		int tileX = courseX / TILE_SIZE;
		int tileY = courseY / TILE_SIZE;

		if (kHeld & KEY_DDOWN) {
			holeX = tileX * TILE_SIZE;
			holeY = tileY * TILE_SIZE;
		} else if (kHeld & KEY_DUP) {
			projX = tileX * TILE_SIZE + (TILE_SIZE / 2);
			projY = tileY * TILE_SIZE + (TILE_SIZE / 2);
		} else if (brushType == RECTANGLE) {
			int tileX2 = (TouchInput_GetSwipe().start.px + scroll)
					/ TILE_SIZE;
			int tileY2 = (TouchInput_GetSwipe().start.py)
					/ TILE_SIZE;
			int startX = tileX > tileX2 ? tileX2 : tileX;
			int endX = tileX > tileX2 ? tileX : tileX2;
			int startY = tileY > tileY2 ? tileY2 : tileY;
			int endY = tileY > tileY2 ? tileY : tileY2;

			for (int x = startX; x <= endX; x++) {
				for (int y = startY; y <= endY; y++) {
					changeTile(x, y, TileSelector_GetTile());
				}
			}
		} else if (brushType == PENCIL) {
			changeTile(tileX, tileY, TileSelector_GetTile());
		}
	}

	return true;
}

// ignored param is to match the signature of Dispatcher_Handler
static bool handleTouchInputRightMenu(void *ignored) {
	if (!rightMenuOpen) return false;

	if (TouchInput_InProgress()) {
		TouchInput_Swipe touch = TouchInput_GetSwipe();
		if (touch.start.px >= RIGHT_MENU_X - BORDER_WIDTH
				&& touch.start.px < RIGHT_MENU_X
					+ RIGHT_MENU_WIDTH + BORDER_WIDTH
				&& touch.start.py >= RIGHT_MENU_Y - BORDER_WIDTH
				&& touch.start.py < RIGHT_MENU_Y
					+ RIGHT_MENU_HEIGHT + BORDER_WIDTH) {
			return true;
		}
	}

	return false;
}

// ignored param is to match the signature of Button callback
static void toggleRightMenu(void *ignored) {
	rightMenuOpen = !rightMenuOpen;
	if (rightMenuOpen) {
		Button_Disable(showDetailsButton);
		Button_Enable(hideDetailsButton);
		Button_Enable(editNameButton);
		Button_Enable(saveButton);
		Button_Enable(exitButton);
		Button_Enable(parUpButton);
		Button_Enable(parDownButton);
	} else {
		Button_Disable(hideDetailsButton);
		Button_Enable(showDetailsButton);
		Button_Disable(editNameButton);
		Button_Disable(saveButton);
		Button_Disable(exitButton);
		Button_Disable(parUpButton);
		Button_Disable(parDownButton);
	}
}

// ignored param is to match the signature of Button callback
static void editName(void *ignored) {
	SwkbdState keyboard;
	SwkbdButton pressedButton;
	char buf[EDITOR_LEVEL_NAME_MAX + 1];
	swkbdInit(&keyboard, SWKBD_TYPE_QWERTY, 2, EDITOR_LEVEL_NAME_MAX);
	swkbdSetValidation(&keyboard, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetHintText(&keyboard, "Enter a name");
	swkbdSetInitialText(&keyboard, name);

	pressedButton = swkbdInputText(&keyboard, buf, EDITOR_LEVEL_NAME_MAX + 1);
	if (pressedButton != SWKBD_BUTTON_CONFIRM) {
		return;
	}

	char *newName = realloc(name, sizeof(char) * (strlen(buf) + 1));
	if (!newName) return;
	name = newName;
	strcpy(name, buf);
	Text_SetContent(nameText, name);
}

// ignored param is to match the signature of Button callback
static void saveExit(void *ignored) {
	if (exportLevel()) {
		Scene_SetNext(sceneLevelSelector, LevelSelector_MakeParams());
		return;
	} else {
		//TODO Figure out better solution than kicking user out
		Scene_SetNext(sceneError, Error_MakeParams("Failed to save file"));
		return;
	}
}

// ignored param is to match the signature of Button callback
static void exitNoSave(void *ignored) {
	Scene_SetNext(sceneLevelSelector, LevelSelector_MakeParams());
}

static void changePar(int change) {
	par += change;
}

static void sceneUpdate() {
	if (BG_IsUpdating(bg)) return;

	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	if (kHeld & KEY_CPAD_LEFT || kHeld & KEY_CSTICK_LEFT)
		scroll -= SCROLL_UNIT;
	if (kHeld & KEY_CPAD_RIGHT || kHeld & KEY_CSTICK_RIGHT)
		scroll += SCROLL_UNIT;
	scroll = clamp(scroll, 0, LEVEL_MAX_WIDTH - 320);

	if (kDown & KEY_DLEFT) par--;
	if (kDown & KEY_DRIGHT) par++;

	if (kDown & KEY_L || kDown & KEY_R) {
		brushType = brushType == PENCIL ? RECTANGLE : PENCIL;
	}	

	Dispatcher_DispatchEvent(touchDispatcher);

	Text_SetContent(parText, "Par %i", par);
}

static void drawRectOutline(int x, int y, int width, int height, u32 color, int 		outlineWidth) {
	C2D_DrawRectSolid(x, y, 0, width, outlineWidth, color);
	C2D_DrawRectSolid(x, y, 0, outlineWidth, height, color);
	C2D_DrawRectSolid(x, y + height - outlineWidth, 0, width, outlineWidth,
			color);
	C2D_DrawRectSolid(x + width - outlineWidth, y, 0, outlineWidth, height,
			color);
}

static void drawRightMenu() {
	Button_Draw(hideDetailsButton, 1);
	Border_Draw(RIGHT_MENU_X, RIGHT_MENU_Y, 0.7, RIGHT_MENU_WIDTH,
			RIGHT_MENU_HEIGHT);
	C2D_DrawRectSolid(RIGHT_MENU_X, RIGHT_MENU_Y, 0.7, RIGHT_MENU_WIDTH,
			RIGHT_MENU_HEIGHT, COLOR_LGRAY);
	Button_Draw(editNameButton, 0.8);
	Button_Draw(saveButton, 0.8);
	Button_Draw(exitButton, 0.8);
	Text_Draw(editNameText, RIGHT_MENU_BUTTON_X + 10, RIGHT_MENU_BUTTON_Y + 5,
			1, COLOR_LGRAY, 1);
	Text_Draw(saveText, RIGHT_MENU_BUTTON_X + 10,
			RIGHT_MENU_BUTTON_Y + RIGHT_MENU_BUTTON_GAP + 5,
			1, COLOR_LGRAY, 1);
	Text_Draw(exitText, RIGHT_MENU_BUTTON_X + 10,
			RIGHT_MENU_BUTTON_Y + 2*RIGHT_MENU_BUTTON_GAP + 5,
			1, COLOR_LGRAY, 1);
	C2D_DrawImageAt(
			SpriteSheet_GetImage(SPRITE_PAR_LABEL),
			RIGHT_MENU_BUTTON_X + 2,
			RIGHT_MENU_BUTTON_Y + 3*RIGHT_MENU_BUTTON_GAP - 1,
			1,
			NULL, 1, 1
		);
	Button_Draw(parUpButton, 0.8);
	Button_Draw(parDownButton, 0.8);
	Text_Draw(parUpText, RIGHT_MENU_X + RIGHT_MENU_WIDTH - 40,
			RIGHT_MENU_BUTTON_Y + 3*RIGHT_MENU_BUTTON_GAP + 9, 1,
			COLOR_LGRAY, 2);
	Text_Draw(parDownText, RIGHT_MENU_BUTTON_X + 19,
			RIGHT_MENU_BUTTON_Y + 3*RIGHT_MENU_BUTTON_GAP + 9, 1,
			COLOR_LGRAY, 2);
}

static void sceneDraw() {
	BG_UpdateGraphics(bg);
	TileSelector_UpdateGraphics();


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	Text_Draw(nameText, TEXT_MARGIN, LEVEL_NAME_Y, 0, COLOR_DGREEN, 1);
	Text_DrawRight(parText, 390, LEVEL_NAME_Y + TEXT_LINE_HEIGHT, 0,
			COLOR_DGREEN, 1);
	BG_DrawFit(bg, LEVEL_PREVIEW_X, LEVEL_PREVIEW_Y, 0, LEVEL_PREVIEW_WIDTH,
			LEVEL_PREVIEW_HEIGHT);
	Border_Draw(LEVEL_PREVIEW_X, LEVEL_PREVIEW_Y, 0, LEVEL_PREVIEW_WIDTH,
			LEVEL_PREVIEW_HEIGHT);

	Text_Draw(controlsText1, TEXT_MARGIN, CONTROLS_TEXT_Y, 0, COLOR_DGREEN, 1);
	Text_Draw(controlsText2, 160, CONTROLS_TEXT_Y, 0, COLOR_DGREEN, 1);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_WHITE);
	C2D_SceneBegin(bottom);

	C2D_ViewTranslate(-scroll, 0);

	BG_Draw(bg, 0, 0, -1, 1, 1);
	drawRectOutline(holeX, holeY, HOLE_WIDTH, HOLE_HEIGHT, COLOR_DRED, 2);
	SpriteSheet_DrawCentered(SPRITE_BALL, projX, projY, 0.5, 0, false, false);

	C2D_ViewReset();

	TileSelector_Draw(0.5);
	Button_Draw(showDetailsButton, 1);
	if (rightMenuOpen) drawRightMenu();
}

Scene sceneEditor = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

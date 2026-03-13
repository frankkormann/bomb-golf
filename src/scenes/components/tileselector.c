#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "tileselector.h"
#include "../../tile.h"
#include "../../rendering/background.h"
#include "../../rendering/colors.h"
#include "../../util/touchinput.h"

#define HOTBAR_LENGTH NUM_TILES

#define TILE_MARGIN 2
#define HOTBAR_WIDTH (NUM_TILES * (TILE_SIZE + TILE_MARGIN) - TILE_MARGIN)
#define POPUP_HEIGHT (8 * (TILE_SIZE + TILE_MARGIN) - TILE_MARGIN)

static enum { HOTBAR, POPUP_MENU } mode;
static Tile hotbar[HOTBAR_LENGTH];
static size_t selectedHotbarIndex;
static Background hotbarBg;
static Background popupBg;

bool TileSelector_Init(Tile defaultTile) {
	hotbarBg = BG_Create(HOTBAR_WIDTH, TILE_SIZE, COLOR_BLUE);
	if (!hotbarBg) goto fail_hotbarBg;

	popupBg = BG_Create(HOTBAR_WIDTH, POPUP_HEIGHT, COLOR_BLUE);
	if (!popupBg) goto fail_popupBg;

	mode = HOTBAR;

	for (size_t i = 0; i < HOTBAR_LENGTH; i++) {
		hotbar[i] = Tile_Make(i + FIRST_TILE_SPRITE, 0);
		BG_DrawTile(hotbarBg, hotbar[i], i * (TILE_SIZE + TILE_MARGIN), 0,
				false);
		for (int orientation = 0; orientation < 8; orientation++) {
			Tile tile = Tile_Make(i + FIRST_TILE_SPRITE, orientation);
			if (tile == defaultTile) {
				selectedHotbarIndex = i;
			}
			BG_DrawTile(popupBg, tile, i * (TILE_SIZE + TILE_MARGIN),
					orientation * (TILE_SIZE + TILE_MARGIN),
					false);
		}
	}

	return true;

fail_popupBg:
	BG_Free(hotbarBg);
fail_hotbarBg:
	return false;
}

void TileSelector_Exit() {
	if (hotbarBg) BG_Free(hotbarBg);
	if (popupBg) BG_Free(popupBg);
}

// Returns true if a touch input was detected and handled
// ignored param is to match the signature of Dispatcher_Handler
static bool handleTouchInput(void *ignored) {
	if (TouchInput_InProgress()) {
		TouchInput_Swipe touch = TouchInput_GetSwipe();
		if (touch.start.px >= 2 && touch.start.px <= HOTBAR_WIDTH
				&& touch.start.py >= 2
				&& touch.start.py <= TILE_SIZE) {
			selectedHotbarIndex = (touch.end.px - 2)
					/ (TILE_SIZE + TILE_MARGIN);
			return true;
		}
	}
	return false;
}

void TileSelector_Update() {
	handleTouchInput(NULL);
}

void TileSelector_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority) {
	Dispatcher_AddHandler(touchDispatcher,
			(Dispatcher_Handler) { priority, handleTouchInput });
}

void TileSelector_UpdateGraphics() {
	BG_UpdateGraphics(hotbarBg);
	BG_UpdateGraphics(popupBg);
}

Tile TileSelector_GetTile() {
	return hotbar[selectedHotbarIndex];
}

bool TileSelector_IsShowingPopup() {
	return mode == POPUP_MENU;
}

static void drawRectOutline(float x, float y, float depth, float width, float height,
		u32 color, int outlineWidth) {
	C2D_DrawRectSolid(x, y, depth, width, outlineWidth, color);
	C2D_DrawRectSolid(x, y, depth, outlineWidth, height, color);
	C2D_DrawRectSolid(x, y + height - outlineWidth, depth, width, outlineWidth,
			color);
	C2D_DrawRectSolid(x + width - outlineWidth, y, depth, outlineWidth, height,
			color);
}

void TileSelector_Draw(float depth) {
	BG_Draw(hotbarBg, 2, 2, depth, 1, 1);
	drawRectOutline(
		selectedHotbarIndex * (TILE_SIZE + TILE_MARGIN) + 1,
		1,
		depth,
		TILE_SIZE + TILE_MARGIN,
		TILE_SIZE + TILE_MARGIN,
		COLOR_DRED, 1
	);
}


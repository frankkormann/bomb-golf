#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "tileselector.h"
#include "button.h"
#include "../../tile.h"
#include "../../rendering/background.h"
#include "../../rendering/colors.h"
#include "../../util/touchinput.h"
#include "../../util/macros.h"

#define HOTBAR_LENGTH NUM_TILES

#define TILE_MARGIN 2
#define ICONS_WIDTH (NUM_TILES * (TILE_SIZE + TILE_MARGIN) - TILE_MARGIN)
#define POPUP_HEIGHT (8 * (TILE_SIZE + TILE_MARGIN) - TILE_MARGIN)

static enum { HOTBAR, POPUP_MENU } mode;
static Tile hotbar[HOTBAR_LENGTH];
static size_t selectedHotbarIndex;
static Background hotbarBg;
static Background popupBg;
static Button expandButton;

static void toggleMode() {
	mode = mode == HOTBAR ? POPUP_MENU : HOTBAR;
}

bool TileSelector_Init(Tile defaultTile) {
	hotbarBg = BG_Create(ICONS_WIDTH, TILE_SIZE, COLOR_BLUE);
	if (!hotbarBg) goto fail_hotbarBg;

	popupBg = BG_Create(ICONS_WIDTH, POPUP_HEIGHT, COLOR_BLUE);
	if (!popupBg) goto fail_popupBg;

	expandButton = Button_Create(ICONS_WIDTH + 2*TILE_MARGIN, TILE_MARGIN,
			SPRITE_BUTTON_EXPAND, toggleMode);
	if (!expandButton) goto fail_expandButton;

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

fail_expandButton:
	BG_Free(popupBg);
fail_popupBg:
	BG_Free(hotbarBg);
fail_hotbarBg:
	return false;
}

void TileSelector_Exit() {
	BG_Free(hotbarBg);
	BG_Free(popupBg);
	Button_Free(expandButton);
}

static bool touchWithinBounds(touchPosition touch) {
	switch (mode) {
		case HOTBAR:
			return touch.px >= TILE_MARGIN
				&& touch.px < TILE_MARGIN + ICONS_WIDTH
				&& touch.py >= TILE_MARGIN
				&& touch.py < TILE_MARGIN + TILE_SIZE;
		case POPUP_MENU:
			return touch.px >= TILE_MARGIN
				&& touch.px < TILE_MARGIN + ICONS_WIDTH
				&& touch.py >= TILE_MARGIN
				&& touch.py < 2*TILE_MARGIN+TILE_SIZE + POPUP_HEIGHT;
	}
	return false;
}

static bool handleTouchInputHotbar() {
	TouchInput_Swipe touch = TouchInput_GetSwipe();
	if (!touchWithinBounds(touch.start)) return false;

	if (touchWithinBounds(touch.end)) {
		selectedHotbarIndex = (touch.end.px - TILE_MARGIN)
				/ (TILE_SIZE + TILE_MARGIN);
	}

	return true;
}

static bool handleTouchInputPopup() {
	TouchInput_Swipe touch = TouchInput_GetSwipe();
	if (!touchWithinBounds(touch.start)) return false;

	if (TouchInput_JustFinished() && touchWithinBounds(touch.end)) {
		selectedHotbarIndex = (touch.end.px - TILE_MARGIN)
				/ (TILE_SIZE + TILE_MARGIN);
		Tile selectedTile = Tile_Make(
				selectedHotbarIndex + FIRST_TILE_SPRITE,
				(touch.end.py - 2*TILE_MARGIN - TILE_SIZE)
					/ (TILE_SIZE + TILE_MARGIN)
			);
		hotbar[selectedHotbarIndex] = selectedTile;
		BG_DrawTile(hotbarBg, selectedTile,
				selectedHotbarIndex * (TILE_MARGIN + TILE_SIZE), 0,
				true);
		mode = HOTBAR;
	}

	return true;
}

// Returns true if a touch input was detected and handled
// ignored param is to match the signature in Dispatcher_Handler
static bool handleTouchInput(void *ignored) {
	if (TouchInput_InProgress() || TouchInput_JustFinished()) {
		return mode == HOTBAR ? handleTouchInputHotbar()
				: handleTouchInputPopup();
	}
	return false;
}

bool TileSelector_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority) {
	if (!Dispatcher_AddHandler(touchDispatcher,
			(Dispatcher_Handler) { priority, NULL, handleTouchInput })) {
		return false;
	}
	if (!Button_RegisterForTouchEvents(expandButton, touchDispatcher,
			priority)) {
		TileSelector_RemoveFromTouchDispatcher(touchDispatcher);
		return false;
	}
	return true;
}

void TileSelector_RemoveFromTouchDispatcher(Dispatcher touchDispatcher) {
	Dispatcher_RemoveHandler(touchDispatcher,
			(Dispatcher_Handler) { 0, NULL, handleTouchInput });
}

void TileSelector_UpdateGraphics() {
	BG_UpdateGraphics(hotbarBg);
	BG_UpdateGraphics(popupBg);
}

Tile TileSelector_GetTile() {
	return hotbar[selectedHotbarIndex];
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

static void drawHotbar(float depth) {
	BG_Draw(hotbarBg, TILE_MARGIN, TILE_MARGIN, depth, 1, 1);
	drawRectOutline(
			selectedHotbarIndex * (TILE_SIZE + TILE_MARGIN) + 1,	
			1,
			depth,
			TILE_SIZE + TILE_MARGIN,
			TILE_SIZE + TILE_MARGIN,
			COLOR_DRED, 1
		);
}

static void drawPopup(float depth) {
	drawHotbar(depth);
	BG_Draw(popupBg, TILE_MARGIN, TILE_SIZE + 2*TILE_MARGIN, depth, 1, 1);
}

void TileSelector_Draw(float depth) {
	mode == HOTBAR ? drawHotbar(depth) : drawPopup(depth);
	Button_Draw(expandButton, depth);
}

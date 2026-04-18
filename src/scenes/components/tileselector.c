#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <citro2d.h>
#include "tileselector.h"
#include "button.h"
#include "background.h"
#include "../../tile.h"
#include "../../rendering/colors.h"
#include "../../util/touchinput.h"
#include "../../util/macros.h"

#define HOTBAR_LENGTH 21

#define BORDER_SIZE 2
#define TILE_GAP 3

#define HOTBAR_X BORDER_SIZE
#define HOTBAR_Y BORDER_SIZE
#define HOTBAR_WIDTH (HOTBAR_LENGTH * (TILE_SIZE + TILE_GAP) - TILE_GAP)
#define HOTBAR_HEIGHT TILE_SIZE

#define HOTBAR_OVERLAY_X 0
#define HOTBAR_OVERLAY_Y 0
#define HOTBAR_OVERLAY_WIDTH (HOTBAR_WIDTH + 2*BORDER_SIZE)
#define HOTBAR_OVERLAY_HEIGHT (TILE_SIZE + 2*BORDER_SIZE)

#define POPUP_X BORDER_SIZE
#define POPUP_Y (HOTBAR_Y + HOTBAR_HEIGHT + BORDER_SIZE + TILE_GAP)
#define POPUP_WIDTH (NUM_TILES * (TILE_SIZE + TILE_GAP) - TILE_GAP)
#define POPUP_HEIGHT (8 * (TILE_SIZE + TILE_GAP) - TILE_GAP)

static enum { HIDDEN, HOTBAR, POPUP_MENU } mode;

static Tile hotbar[HOTBAR_LENGTH];
static size_t selectedHotbarIndex;
static Background hotbarBg;

static Background popupBg;

static Button buttonExpand;
static Button buttonShrink;

static void handleExpand(void *ignored) {
	mode = mode == HIDDEN ? HOTBAR
	     : mode == HOTBAR ? POPUP_MENU
	     : mode;
}
static void handleShrink(void *ignored) {
	mode = mode == POPUP_MENU ? HOTBAR
	     : mode == HOTBAR ? HIDDEN
	     : mode;
}

static void setHotbarTile(Tile tile, int index) {
	hotbar[index] = tile;
	BG_DrawTile(hotbarBg, tile, index * (TILE_SIZE + TILE_GAP), 0, true);
}

bool TileSelector_Init(Tile defaultTile) {
	hotbarBg = BG_Create(HOTBAR_WIDTH, TILE_SIZE, COLOR_BLUE);
	if (!hotbarBg) goto fail_hotbarBg;

	popupBg = BG_Create(POPUP_WIDTH, POPUP_HEIGHT, COLOR_BLUE);
	if (!popupBg) goto fail_popupBg;

	buttonExpand = Button_Create(HOTBAR_X + HOTBAR_WIDTH + 5, HOTBAR_Y,
			SPRITE_BUTTON_DOWN, NULL, handleExpand);
	if (!buttonExpand) goto fail_buttonExpand;

	buttonShrink = Button_Create(HOTBAR_X + HOTBAR_WIDTH + 23, HOTBAR_Y,
			SPRITE_BUTTON_UP, NULL, handleShrink);
	if (!buttonShrink) goto fail_buttonShrink;

	mode = HOTBAR;

	for (size_t i = 0; i < NUM_TILES; i++) {
		if (i < HOTBAR_LENGTH) setHotbarTile(Tile_Make(i, 0), i);
		for (int orientation = 0; orientation < 8; orientation++) {
			Tile tile = Tile_Make(i, orientation);
			if (tile == defaultTile) {
				selectedHotbarIndex = i;
			}
			BG_DrawTile(
					popupBg,
					tile,
					i * (TILE_SIZE + TILE_GAP),
					orientation * (TILE_SIZE + TILE_GAP),
					false
				);
		}
	}

	for (size_t i = NUM_TILES; i < HOTBAR_LENGTH; i++) {
		hotbar[i] = Tile_Make(SPRITE_TILE_SKY, 0);
		BG_DrawTile(
				hotbarBg,
				hotbar[i],
				i * (TILE_SIZE + TILE_GAP),
				0,
				false
			);
	}

	return true;

fail_buttonShrink:
	Button_Free(buttonExpand);
fail_buttonExpand:
	BG_Free(popupBg);
fail_popupBg:
	BG_Free(hotbarBg);
fail_hotbarBg:
	return false;
}

void TileSelector_Exit() {
	BG_Free(hotbarBg);
	BG_Free(popupBg);
	Button_Free(buttonExpand);
	Button_Free(buttonShrink);
}

static bool touchWithinBounds(touchPosition touch, float x, float y, float width,
		float height) {
	return     touch.px >= x && touch.px < x + width
	        && touch.py >= y && touch.py < y + height;
}

static bool handleTouchInputHotbar() {
	TouchInput_Swipe touch = TouchInput_GetSwipe();
	if (!touchWithinBounds(touch.start, HOTBAR_OVERLAY_X, HOTBAR_OVERLAY_Y,
			HOTBAR_OVERLAY_WIDTH, HOTBAR_OVERLAY_HEIGHT)) {
		return false;
	}

	if (touchWithinBounds(touch.start, HOTBAR_X, HOTBAR_Y, HOTBAR_WIDTH,
			HOTBAR_HEIGHT)) {
		selectedHotbarIndex = (touch.end.px - HOTBAR_X)
				/ (TILE_SIZE + TILE_GAP);
	}

	return true;
}

static bool handleTouchInputPopup() {
	TouchInput_Swipe touch = TouchInput_GetSwipe();
	if (!touchWithinBounds(touch.start, POPUP_X,POPUP_Y, POPUP_WIDTH,
			POPUP_HEIGHT)
		&& !touchWithinBounds(touch.start, HOTBAR_X, HOTBAR_Y,
				HOTBAR_WIDTH, HOTBAR_HEIGHT)) {
		return handleTouchInputHotbar();
	}

	if (TouchInput_JustFinished() && touchWithinBounds(touch.end, POPUP_X,
			POPUP_Y, POPUP_WIDTH, POPUP_HEIGHT)) {
		int tileIndex = (touch.end.px - POPUP_X) / (TILE_SIZE + TILE_GAP);
		int tileOrient = (touch.end.py - POPUP_Y) / (TILE_SIZE + TILE_GAP);
		setHotbarTile(Tile_Make(tileIndex, tileOrient), selectedHotbarIndex);
		mode = HOTBAR;
	}

	return true;
}

// Returns true if a touch input was detected and handled
// ignored param is to match the signature in Dispatcher_Handler
static bool handleTouchInput(void *ignored) {
	if (TouchInput_InProgress() || TouchInput_JustFinished()) {
		return    mode == HOTBAR     ? handleTouchInputHotbar()
			: mode == POPUP_MENU ? handleTouchInputPopup()
			: false;
	}
	return false;
}

bool TileSelector_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority) {
	if (!Dispatcher_AddHandler(touchDispatcher,
			(Dispatcher_Handler) { priority, NULL, handleTouchInput })) {
		return false;
	}
	if (!Button_RegisterForTouchEvents(buttonExpand, touchDispatcher,
			priority)) {
		TileSelector_RemoveFromTouchDispatcher(touchDispatcher);
		return false;
	}
	if (!Button_RegisterForTouchEvents(buttonShrink, touchDispatcher,
			priority)) {
		Button_RemoveFromTouchDispatcher(buttonExpand, touchDispatcher);
		TileSelector_RemoveFromTouchDispatcher(touchDispatcher);
		return false;
	}
	return true;
}

void TileSelector_RemoveFromTouchDispatcher(Dispatcher touchDispatcher) {
	Dispatcher_RemoveHandler(touchDispatcher,
			(Dispatcher_Handler) { 0, NULL, handleTouchInput });
	Button_RemoveFromTouchDispatcher(buttonExpand, touchDispatcher);
	Button_RemoveFromTouchDispatcher(buttonShrink, touchDispatcher);
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
	BG_Draw(hotbarBg, HOTBAR_X, HOTBAR_Y, depth, 1, 1);
	C2D_Image overlay = SpriteSheet_GetImage(SPRITE_HOTBAR_OVERLAY);
	C2D_DrawImageAt(overlay, HOTBAR_OVERLAY_X, HOTBAR_OVERLAY_Y, depth, NULL, 1,
			1);
	drawRectOutline(
			selectedHotbarIndex * (TILE_SIZE + TILE_GAP) + HOTBAR_X - 1,
			HOTBAR_Y - 1,
			depth,
			TILE_SIZE + 2,
			TILE_SIZE + 2,
			COLOR_DRED,
			1
		);
}

static void drawPopup(float depth) {
	drawHotbar(depth);
	BG_Draw(popupBg, POPUP_X, POPUP_Y, depth, 1, 1);
}

void TileSelector_Draw(float depth) {
	if (mode == HOTBAR) drawHotbar(depth);
	if (mode == POPUP_MENU) drawPopup(depth);

	if (mode != HIDDEN) Button_Draw(buttonShrink, depth);
	if (mode != POPUP_MENU) Button_Draw(buttonExpand, depth);
}

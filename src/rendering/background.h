/*
 * Draw persistent backgrounds of many tiles.
 *
 * Tiles and individual pixels are drawn to the Background, then the Background
 * is drawn as a whole to the C3D_RenderTarget. Objects persist on the
 * Background between graphics frames unless overwritten. This allows for more
 * tiles to be effectively drawn to the screen than C2D's max objects.
 *
 * Tiles and pixels to be drawn are queued until a call to BG_UpdateGraphics is
 * made. Therefore, these functions are safe to call outside of rendering.
 */

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <3ds.h>
#include <stdbool.h>

// Width and height
#define BG_TILE_SIZE 10

typedef struct background* Background;

typedef enum {
	TILE_CLEAR,
	TILE_GREEN,
	// Directions are where the angle opposite the hypotenuse is
	TILE_TRI_NW,
	TILE_TRI_NE,
	TILE_TRI_SW,
	TILE_TRI_SE,
	
	TILE_HALF_N,
	TILE_HALF_S,
	TILE_HALF_W,
	TILE_HALF_E,
	NUM_TILES
} BG_Tile;

/*
 * Allocates a new Background of size (width, height). Initially fills it in
 * with clearColor.
 *
 * Returns the Background or NULL if an error occurs.
 */
Background BG_Create(unsigned int width, unsigned int height, u32 clearColor);

void BG_Free(Background bg);

/*
 * Draws tile to bg at (x, y). Transparent pixels in tile's texture will not
 * be cleared unless clearPrevious is true. (x, y) do not have to be multiples
 * of BG_TILE_SIZE.
 *
 * Returns false if an error occurred.
 */
bool BG_DrawTile(Background bg, BG_Tile tile, int x, int y, bool clearPrevious);

/*
 * Overwites the pixel at (x, y) in bg with bg's clear color.
 *
 * Returns false if an error occurred.
 */
bool BG_ClearPixel(Background bg, int x, int y);

/*
 * Clears all of bg's pixels with bg's clear color.
 */
void BG_ClearAll(Background bg);

/*
 * Draws as many queued tiles and pixels as possible. Only call during drawing,
 * that is, during C3D_FrameBegin and C3D_FrameEnd.
 */
void BG_UpdateGraphics(Background bg);

/*
 * Returns true if there are queued tiles or pixels which are not yet drawn.
 */
bool BG_IsUpdating(Background bg);

void BG_Draw(Background bg, float x, float y, float depth, float scaleX,
		float scaleY);

#endif

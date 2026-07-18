/*
 * Part of the environment which the projectile can interact with. Different
 * types of terrain have different properties.
 *
 * Terrain can be filled in from Tiles and cleared using geometric shapes.
 */

#ifndef TERRAIN_H
#define TERRAIN_H

#include <stdbool.h>
#include "../tile.h"

typedef enum {
	TERRAIN_NOTHING,
	TERRAIN_GROUND,
	TERRAIN_BOUNCY,
	TERRAIN_EXPLOSIVE
//	TERRAIN_STICKY,    // Future idea
} Terrain_Type;

/*
 * Creates an environment, initially empty, of size (width, height).
 *
 * Returns false if an error occurs.
 */
bool Terrain_Init(int width, int height);

void Terrain_Exit();

/*
 * (x, y) is the top-left corner of tile. Terrain_Type is inferred.
 */
void Terrain_FillTile(int x, int y, Tile tile, bool clearPrevious);

/*
 * Removes the terrain at (x, y).
 */
void Terrain_ClearPixel(int x, int y);

Terrain_Type Terrain_TypeAt(int x, int y);

/*
 * Call this once per physics frame.
 */
void Terrain_Update();

/*
 * Call this once per graphics frame.
 */
void Terrain_UpdateGraphics();

/*
 * Draws the terrain inside the rectangle with its top-left corner at (x, y)
 * and size (maxWidth, maxHeight).
 *
 * If drawnXYZ is not NULL, fills in its value with the actual position that
 * the terrain was drawn with.
 */
void Terrain_Draw(int x, int y, float depth, int maxWidth, int maxHeight,
		int *drawnX, int *drawnY, int *drawnWidth, int *drawnHeight);

#endif

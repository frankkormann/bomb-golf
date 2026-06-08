/**
 * Make Tiles with a sprite and hitbox. Can be flipped or rotated. Hitbox is
 * inferred from the sprite and orientation.
 *
 * The Tile type contains all non-position information for a Tile. The
 * Tile_WithPos type contains a Tile and position information aligned to a
 * TILE_SIZE grid.
 */

#ifndef TILE_H
#define TILE_H

#include <stdbool.h>
#include <3ds.h>
#include "rendering/spritesheet.h"

// Width and height
#define TILE_SIZE 10

typedef enum {
	/*               *
	 *               *
	 *               *
	 *               */
	TILE_HITBOX_NONE,
	/*    * * * *    *
	 *    * * * *    *
	 *    * * * *    *
	 *    * * * *    */
	TILE_HITBOX_FULL,
	/*    * *        *
	 *    * *        *
	 *    * *        *
	 *    * *        */
	TILE_HITBOX_HALF,
	/*          *    *
	 *        * *    *
	 *      * * *    *
	 *    * * * *    */
	TILE_HITBOX_TRIANGLE
} Tile_Hitbox;

// Flips are applied before rotation
typedef enum {
	TILE_ROTATE_90 = 1,
	TILE_FLIP_HORIZ = 2,
	TILE_FLIP_VERT = 4
} Tile_OrientFlags;

typedef u16 Tile;
typedef u32 Tile_WithPos;

Tile Tile_Make(SpriteSheet_TileSprite sprite, Tile_OrientFlags flags);

/*
 * (x, y) must be from (0, 0) to (LEVEL_HEIGHT, LEVEL_MAX_WIDTH). If (x, y) is
 * not aligned to a TILE_SIZE grid, it will be rounded down.
 */
Tile_WithPos Tile_AddPos(Tile tile, int x, int y);

SpriteSheet_TileSprite Tile_GetSprite(Tile tile);
Tile_Hitbox Tile_GetHitbox(Tile tile);
u8 Tile_GetOrientFlags(Tile tile);

/*
 * Returns true if tile should be layered on top of another Tile.
 */
bool Tile_IsOverlay(Tile tile);

Tile Tile_GetTile(Tile_WithPos tileWithPos);
void Tile_GetPos(Tile_WithPos tileWithPos, int *x, int *y);

#endif

/**
 * Make Tiles with a sprite and hitbox. Can be flipped or rotated. Hitbox is
 * inferred from the sprite and orientation.
 */

#ifndef TILE_H
#define TILE_H

#include <3ds.h>
#include "rendering/spritesheet.h"

// Width and height
#define TILE_SIZE 10

#define NUM_TILES 10
// Index of the first sprite for tiles
#define FIRST_TILE_SPRITE 2

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

Tile Tile_Make(SpriteSheet_Sprite sprite, Tile_OrientFlags flags);

SpriteSheet_Sprite Tile_GetSprite(Tile tile);
Tile_Hitbox Tile_GetHitbox(Tile tile);
u8 Tile_GetOrientFlags(Tile tile);

#endif

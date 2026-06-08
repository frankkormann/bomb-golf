#include <stdbool.h>
#include <3ds.h>
#include "tile.h"
#include "rendering/spritesheet.h"

/*
 * For Tile:
 *    Bits 1-3  are orientation flags,
 *         4-5  are hitbox,
 *           6  is whether the tile is an overlay or not,
 *         7-8  are padding/future use,
 *         9-16 are sprite
 *
 * For Tile_WithPos:
 *    Bits  1-16 are the Tile,
 *         17-24 are x position (in TILE_SIZE grid coordinates),
 *         25-32 are y position (in TILE_SIZE grid coordinates)
 */

Tile_Hitbox spriteToHitbox(SpriteSheet_TileSprite sprite) {
	switch (sprite) {
		case SPRITE_TILE_SKY:
		case SPRITE_TILE_HOUSE_INTERIOR:
		case SPRITE_TILE_OVERLAY_BOUNCY:
		case NUM_TILES:
			return TILE_HITBOX_NONE;
		case SPRITE_TILE_DIRT:
		case SPRITE_TILE_DIRT_INTERNAL:
		case SPRITE_TILE_GRASS:
		case SPRITE_TILE_GRASS_TRIANGLE_FILLER:
		case SPRITE_TILE_TREE:
		case SPRITE_TILE_LEAVES:
		case SPRITE_TILE_HOLE_WALL:
		case SPRITE_TILE_HOLE_WALL_TRIANGLE:
		case SPRITE_TILE_HOUSE_WALL:
		case SPRITE_TILE_HOUSE_FLOOR:
		case SPRITE_TILE_ROOF:
		case SPRITE_TILE_ROOF_WALL:
			return TILE_HITBOX_FULL;
		case SPRITE_TILE_DIRT_TRIANGLE:
		case SPRITE_TILE_GRASS_TRIANGLE_1:
		case SPRITE_TILE_GRASS_TRIANGLE_2:
		case SPRITE_TILE_ROOF_TRIANGLE_INTERIOR:
		case SPRITE_TILE_ROOF_TRIANGLE_EXTERIOR:
			return TILE_HITBOX_TRIANGLE;
		case SPRITE_TILE_GRASS_HALF:
		case SPRITE_TILE_DIRT_HALF:
		case SPRITE_TILE_LEAVES_HALF:
			return TILE_HITBOX_HALF;
	}
	return TILE_HITBOX_NONE;
}

u8 isOverlay(SpriteSheet_TileSprite sprite) {
	if (sprite == SPRITE_TILE_OVERLAY_BOUNCY) {
		return 1;
	} else {
		return 0;
	}
}

Tile Tile_Make(SpriteSheet_TileSprite sprite, Tile_OrientFlags flags) {
	return flags | (spriteToHitbox(sprite) << 3) | (isOverlay(sprite) << 5)
			| (sprite << 8);
}

Tile_WithPos Tile_AddPos(Tile tile, int x, int y) {
	u8 tileX = x / TILE_SIZE;
	u8 tileY = y / TILE_SIZE;
	return tile | (tileX << 16) | (tileY << 24);
}

SpriteSheet_TileSprite Tile_GetSprite(Tile tile) {
	return ((tile >> 8) & 0xFF);
}

Tile_Hitbox Tile_GetHitbox(Tile tile) {
	return (tile >> 3) & 0x3;
}

u8 Tile_GetOrientFlags(Tile tile) {
	return tile & 0x7;
}

bool Tile_IsOverlay(Tile tile) {
	return ((tile >> 5) & 0x1) == 1;
}

Tile Tile_GetTile(Tile_WithPos tileWithPos) {
	return tileWithPos & 0xFFFF;
}

void Tile_GetPos(Tile_WithPos tileWithPos, int *x, int *y) {
	u8 tileX = (tileWithPos >> 16) & 0xFF;
	u8 tileY = (tileWithPos >> 24) & 0xFF;
	*x = tileX * TILE_SIZE;
	*y = tileY * TILE_SIZE;
}

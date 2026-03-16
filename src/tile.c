#include <3ds.h>
#include "tile.h"
#include "rendering/spritesheet.h"

Tile_Hitbox spriteToHitbox(SpriteSheet_TileSprite sprite) {
	switch (sprite) {
		case SPRITE_TILE_SKY:
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
			return TILE_HITBOX_FULL;
		case SPRITE_TILE_DIRT_TRIANGLE:
		case SPRITE_TILE_GRASS_TRIANGLE_1:
		case SPRITE_TILE_GRASS_TRIANGLE_2:
			return TILE_HITBOX_TRIANGLE;
		case SPRITE_TILE_GRASS_HALF:
		case SPRITE_TILE_DIRT_HALF:
		case SPRITE_TILE_LEAVES_HALF:
			return TILE_HITBOX_HALF;
	}
	return TILE_HITBOX_NONE;
}

// Bits 1-3 are orientation flags, 4-5 are hitbox, 6-8 are padding/future use,
// and 8-16 are sprite

Tile Tile_Make(SpriteSheet_TileSprite sprite, Tile_OrientFlags flags) {
	return flags | (spriteToHitbox(sprite) << 3) | (sprite << 8);
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

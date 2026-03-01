#include <3ds.h>
#include "tile.h"
#include "rendering/spritesheet.h"

Tile_Hitbox spriteToHitbox(SpriteSheet_Sprite sprite) {
	switch (sprite) {
		case SPRITE_BALL:
		case SPRITE_BOMB:
		case SPRITE_SKY:
			return TILE_HITBOX_NONE;
		case SPRITE_DIRT:
		case SPRITE_DIRT_INTERNAL:
		case SPRITE_DIRT_TRIANGLE_FILLER:
		case SPRITE_GRASS:
		case SPRITE_GRASS_TRIANGLE_FILLER:
		case SPRITE_TREE:
		case SPRITE_LEAVES:
			return TILE_HITBOX_FULL;
		case SPRITE_DIRT_TRIANGLE:
		case SPRITE_GRASS_TRIANGLE_1:
		case SPRITE_GRASS_TRIANGLE_2:
			return TILE_HITBOX_TRIANGLE;
		case SPRITE_GRASS_HALF:
			return TILE_HITBOX_HALF;
	}
	return TILE_HITBOX_NONE;
}

// Bits 1-3 are orientation flags, 4-5 are hitbox, 6-8 are padding/future use,
// and 8-16 are sprite

Tile Tile_Make(SpriteSheet_Sprite sprite, Tile_OrientFlags flags) {
	return flags | (spriteToHitbox(sprite) << 3) | (sprite << 8);
}

SpriteSheet_Sprite Tile_GetSprite(Tile tile) {
	return (tile >> 8) & 0xFF;
}

Tile_Hitbox Tile_GetHitbox(Tile tile) {
	return (tile >> 3) & 0x3;
}

u8 Tile_GetOrientFlags(Tile tile) {
	return tile & 0x7;
}

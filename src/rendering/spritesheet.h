/*
 * Get sprites as C2D_Images or draw them directly.
 */

#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <citro2d.h>

typedef enum {
	SPRITE_BALL,
	SPRITE_BOMB,

	SPRITE_FIREWORK_1,
	SPRITE_FIREWORK_2,
	SPRITE_FIREWORK_3,
	SPRITE_FIREWORK_4,
	SPRITE_FIREWORK_5,
	SPRITE_FIREWORK_6,
	SPRITE_FIREWORK_7,
	SPRITE_FIREWORK_8,
	SPRITE_FIREWORK_9,
	SPRITE_FIREWORK_10,
	SPRITE_FIREWORK_11,
	SPRITE_FIREWORK_12,

	SPRITE_BUTTON_UP,
	SPRITE_BUTTON_DOWN,
	SPRITE_HOTBAR_OVERLAY,
	SPRITE_TITLE,
	SPRITE_TITLE_BUTTON,
	SPRITE_LEVEL_CARD_BACKGROUND,
	SPRITE_LEVEL_EDIT_BUTTON,
	SPRITE_LEVEL_PLAY_BUTTON,
	SPRITE_LEVEL_PLAY_BUTTON_DISABLED,
	SPRITE_GUI_BORDER,
	SPRITE_GUI_BORDER_CORNER
} SpriteSheet_Sprite;

// These values are written directly into level files, so be careful when
// reassigning or removing them
typedef enum {
	SPRITE_TILE_DIRT,
	SPRITE_TILE_DIRT_INTERNAL,
	SPRITE_TILE_DIRT_TRIANGLE,
	SPRITE_TILE_DIRT_HALF,
	SPRITE_TILE_GRASS,
	SPRITE_TILE_GRASS_HALF,
	SPRITE_TILE_GRASS_TRIANGLE_1,
	SPRITE_TILE_GRASS_TRIANGLE_2,
	SPRITE_TILE_GRASS_TRIANGLE_FILLER,
	SPRITE_TILE_SKY,
	SPRITE_TILE_TREE,
	SPRITE_TILE_LEAVES,
	SPRITE_TILE_LEAVES_HALF,
	SPRITE_TILE_HOLE_WALL,
	SPRITE_TILE_HOLE_WALL_TRIANGLE,

	NUM_TILES
} SpriteSheet_TileSprite;

/*
 * Returns false on failure.
 */
bool SpriteSheet_Init();

void SpriteSheet_Exit();

C2D_Image SpriteSheet_GetImage(SpriteSheet_Sprite sprite);

/*
 * For SPRITE_BOMB, the "center" is offset vertically to match the visual
 * center of mass.
 */
void SpriteSheet_DrawCentered(SpriteSheet_Sprite sprite, float x, float y,
		float depth, float angle, bool flipHoriz, bool flipVert);

void SpriteSheet_Draw(SpriteSheet_Sprite sprite, float x, float y, float depth,
		float angle, bool flipHoriz, bool flipVert);

/*
 * Note: (x, y) is the top-left corner of the tile
 */
void SpriteSheet_DrawTile(SpriteSheet_TileSprite tile, float x, float y, float depth,
		float angle, bool flipHoriz, bool flipVert);

#endif

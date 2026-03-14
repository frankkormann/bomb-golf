/*
 * Get sprites as C2D_Images or draw them directly.
 */

#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <citro2d.h>

// These values are written directly into level files, so be careful when
// reassigning or removing them
typedef enum {
	SPRITE_BALL,
	SPRITE_BOMB,
	SPRITE_DIRT,
	SPRITE_DIRT_INTERNAL,
	SPRITE_DIRT_TRIANGLE,
	SPRITE_DIRT_HALF,
	SPRITE_GRASS,
	SPRITE_GRASS_HALF,
	SPRITE_GRASS_TRIANGLE_1,
	SPRITE_GRASS_TRIANGLE_2,
	SPRITE_GRASS_TRIANGLE_FILLER,
	SPRITE_SKY,
	SPRITE_TREE,
	SPRITE_LEAVES,
	SPRITE_LEAVES_HALF,
	SPRITE_HOLE_WALL,
	SPRITE_HOLE_WALL_TRIANGLE,

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

	SPRITE_BUTTON_EXPAND
} SpriteSheet_Sprite;

void SpriteSheet_Init();
void SpriteSheet_Exit();

C2D_Image SpriteSheet_GetImage(SpriteSheet_Sprite sprite);

void SpriteSheet_Draw(SpriteSheet_Sprite sprite, float x, float y, float depth,
		float angle, bool flipHoriz, bool flipVert, C2D_ImageTint *tint);

#endif

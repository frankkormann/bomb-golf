/*
 * Get sprites as C2D_Images or draw them directly.
 */

#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <citro2d.h>

typedef enum {
	SPRITE_BALL,
	SPRITE_BOMB,
	SPRITE_DIRT,
	SPRITE_DIRT_INTERNAL,
	SPRITE_DIRT_TRIANGLE,
	SPRITE_DIRT_TRIANGLE_FILLER,
	SPRITE_GRASS,
	SPRITE_GRASS_HALF,
	SPRITE_GRASS_TRIANGLE_1,
	SPRITE_GRASS_TRIANGLE_2,
	SPRITE_GRASS_TRIANGLE_FILLER,
	SPRITE_SKY
} SpriteSheet_Sprite;

void SpriteSheet_Init();
void SpriteSheet_Exit();

C2D_Image SpriteSheet_GetImage(SpriteSheet_Sprite sprite);

void SpriteSheet_Draw(SpriteSheet_Sprite sprite, float x, float y, float depth,
		float angle, bool flipHoriz, bool flipVert, C2D_ImageTint *tint);

#endif

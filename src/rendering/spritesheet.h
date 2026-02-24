/*
 * Get sprites as C2D_Images or draw them directly.
 */

#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <citro2d.h>

typedef enum {
	SPRITE_BALL	= 0,
	SPRITE_BOMB	= 1,
	SPRITE_TRIANGLE	= 2,
	SPRITE_BLOCK	= 3,
	SPRITE_HALF	= 4
} SpriteSheet_Sprite;

void SpriteSheet_Init();
void SpriteSheet_Exit();

C2D_Image SpriteSheet_GetImage(SpriteSheet_Sprite sprite);

void SpriteSheet_Draw(SpriteSheet_Sprite sprite, float x, float y, float depth,
		float angle);

/*
 * Like SpriteSheet_Draw but each of the sprite's opaque pixels is replaced by
 * newColor.
 */
void SpriteSheet_DrawOverwriteColor(SpriteSheet_Sprite sprite, float x, float y,
		float depth, float angle, u32 newColor);

#endif

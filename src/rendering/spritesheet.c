#include <3ds.h>
#include <citro2d.h>
#include "spritesheet.h"

static const float centers[][2] = {
	{ 0.5, 0.5 },	// SPRITE_BALL
	{ 0.5, 0.75 },	// SPRITE_BOMB
	{ 0.5, 0.5 },	// SPRITE_TRIANGLE
	{ 0.5, 0.5 },	// SPRITE_BLOCK
	{ 0.5, 0.5 },	// SPRITE_HALF
};

static C2D_SpriteSheet spriteSheet;

void SpriteSheet_Init() {
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
}

void SpriteSheet_Exit() {
	C2D_SpriteSheetFree(spriteSheet);
}

C2D_Image SpriteSheet_GetImage(SpriteSheet_Sprite sprite) {
	return C2D_SpriteSheetGetImage(spriteSheet, sprite);
}

void drawSprite(SpriteSheet_Sprite sprite, float x, float y, float depth,
		float angle, C2D_ImageTint *tint) {
	C2D_Image img = SpriteSheet_GetImage(sprite);
	C2D_DrawImage(img, &(C2D_DrawParams) {
		.pos = { x, y, img.subtex->width, img.subtex->height },
		.center = {
			img.subtex->width * centers[sprite][0],
			img.subtex->height * centers[sprite][1]
		},
		.depth = depth,
		.angle = angle
	}, tint);
}

void SpriteSheet_Draw(SpriteSheet_Sprite sprite, float x, float y, float depth,
		float angle) {
	drawSprite(sprite, x, y, depth, angle, NULL);
}

void SpriteSheet_DrawOverwriteColor(SpriteSheet_Sprite sprite, float x, float y,
		float depth, float angle, u32 newColor) {
	C2D_ImageTint tint;
	C2D_PlainImageTint(&tint, newColor, 1);
	drawSprite(sprite, x, y, depth, angle, &tint);
}

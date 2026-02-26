#include <3ds.h>
#include <citro2d.h>
#include "spritesheet.h"

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

void SpriteSheet_Draw(SpriteSheet_Sprite sprite, float x, float y, float depth,
		float angle, bool flipHoriz, bool flipVert, C2D_ImageTint *tint) {
	C2D_Image img = SpriteSheet_GetImage(sprite);

	C2D_DrawImage(img, &(C2D_DrawParams) {
		.pos = {
			x,
			y,
			img.subtex->width * (flipHoriz ? -1 : 1),
			img.subtex->height * (flipVert ? -1 : 1)
		},
		.center = {
			img.subtex->width / 2,
			sprite == SPRITE_BOMB ? 10 : img.subtex->height / 2
		},
		.depth = depth,
		.angle = angle
	}, tint);
}

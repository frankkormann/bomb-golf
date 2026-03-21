#include <3ds.h>
#include <citro2d.h>
#include "spritesheet.h"

static C2D_SpriteSheet spriteSheet;
static C2D_SpriteSheet tileSheet;

void SpriteSheet_Init() {
	spriteSheet = C2D_SpriteSheetLoad("romfs:/gfx/sprites.t3x");
	tileSheet = C2D_SpriteSheetLoad("romfs:/gfx/tiles.t3x");
}

void SpriteSheet_Exit() {
	C2D_SpriteSheetFree(spriteSheet);
	C2D_SpriteSheetFree(tileSheet);
}

C2D_Image SpriteSheet_GetImage(SpriteSheet_Sprite sprite) {
	return C2D_SpriteSheetGetImage(spriteSheet, sprite);
}

void SpriteSheet_Draw(SpriteSheet_Sprite sprite, float x, float y, float depth,
		float angle, bool flipHoriz, bool flipVert) {
	C2D_Image img = C2D_SpriteSheetGetImage(spriteSheet, sprite);
	C2D_DrawImage(img, &(C2D_DrawParams) {
		.pos = {
			x,
			y,
			img.subtex->width * (flipHoriz ? -1 : 1),
			img.subtex->height * (flipVert ? -1 : 1)
		},
		.center = {
			img.subtex->width * 0.5,
			sprite == SPRITE_BOMB ? 10 : img.subtex->height * 0.5
		},
		.depth = depth,
		.angle = angle
	}, NULL);
}

void SpriteSheet_DrawTile(SpriteSheet_TileSprite tile, float x, float y, float depth,
		float angle, bool flipHoriz, bool flipVert) {
	// We set the center to the tile's real center and offset its position so
	// that its top-left corner is still (x, y). This way rotation works as
	// expected (rotating about the center).
	C2D_Image img = C2D_SpriteSheetGetImage(tileSheet, tile);
	C2D_DrawImage(img, &(C2D_DrawParams) {
		.pos = {
			x + img.subtex->width * 0.5,
			y + img.subtex->height * 0.5,
			img.subtex->width * (flipHoriz ? -1 : 1),
			img.subtex->height * (flipVert ? -1 : 1)
		},
		.center = {
			img.subtex->width * 0.5,
			img.subtex->height * 0.5
		},
		.depth = depth,
		.angle = angle
	}, NULL);
}

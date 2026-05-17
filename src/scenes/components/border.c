#include <citro2d.h>
#include "border.h"
#include "../../rendering/spritesheet.h"
#include "../../util/macros.h"

#define BORDER_TILE_SIZE 10

// If x, y, width, height are floats, the drawing messes up sometimes
void Border_Draw(int x, int y, float depth, int width, int height) {

	for (int cx = x; cx + BORDER_TILE_SIZE < x + width; cx += BORDER_TILE_SIZE) {
		SpriteSheet_Draw(SPRITE_GUI_BORDER,
				cx, y - BORDER_TILE_SIZE,
				depth, M_PI/2, true, false);
		SpriteSheet_Draw(SPRITE_GUI_BORDER,
				cx, y + height,
				depth, M_PI/2, false, false);
	}
	SpriteSheet_Draw(SPRITE_GUI_BORDER,
			x + width - BORDER_TILE_SIZE, y - BORDER_TILE_SIZE,
			depth, M_PI/2, true, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER,
			x + width - BORDER_TILE_SIZE, y + height,
			depth, M_PI/2, false, false);
	for (int cy = y; cy + BORDER_TILE_SIZE < y + height; cy += BORDER_TILE_SIZE) {
		SpriteSheet_Draw(SPRITE_GUI_BORDER,
				x - BORDER_TILE_SIZE, cy,
				depth, 0, true, false);
		SpriteSheet_Draw(SPRITE_GUI_BORDER,
				x + width, cy,
				depth, 0, false, false);
	}
	SpriteSheet_Draw(SPRITE_GUI_BORDER,
			x - BORDER_TILE_SIZE, y + height - BORDER_TILE_SIZE,
			depth, 0, true, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER,
			x + width, y + height - BORDER_TILE_SIZE,
			depth, 0, false, false);
/*
	for (float cx = x; cx < x + width; cx += BORDER_TILE_SIZE) {
		SpriteSheet_Draw(SPRITE_GUI_BORDER,
				min(cx, x + width - BORDER_TILE_SIZE),
				y - BORDER_TILE_SIZE, depth,
				M_PI/2, true, false);
		SpriteSheet_Draw(SPRITE_GUI_BORDER,
				min(cx, x + width - BORDER_TILE_SIZE),
				y + height, depth,
				M_PI/2, false, false);
	}
	for (float cy = y; cy < y + height; cy += BORDER_TILE_SIZE) {
		SpriteSheet_Draw(SPRITE_GUI_BORDER, x - BORDER_TILE_SIZE,
				min(cy, y + height - BORDER_TILE_SIZE), depth,
				0, true, false);
		SpriteSheet_Draw(SPRITE_GUI_BORDER, x + width,
				min(cy, y + height - BORDER_TILE_SIZE), depth,
				0, false, false);
	}
*/
	SpriteSheet_Draw(SPRITE_GUI_BORDER_CORNER,
			x - BORDER_TILE_SIZE, y - BORDER_TILE_SIZE,
			depth, 0, false, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_CORNER,
			x + width, y - BORDER_TILE_SIZE,
			depth, M_PI/2, false, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_CORNER,
			x - BORDER_TILE_SIZE, y + height,
			depth, M_PI/2, true, true);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_CORNER,
			x + width, y + height,
			depth, 0, true, true);
}

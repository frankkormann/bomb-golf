#include <citro2d.h>
#include "border.h"
#include "../../rendering/spritesheet.h"
#include "../../util/macros.h"

#define BORDER_TILE_SIZE 10

void Border_Draw(float x, float y, float depth, float width, float height) {
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
	SpriteSheet_Draw(SPRITE_GUI_BORDER_CORNER,
			x - BORDER_TILE_SIZE, y - BORDER_TILE_SIZE, depth,
			0, false, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_CORNER,
			x + width, y - BORDER_TILE_SIZE, depth,
			M_PI/2, false, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_CORNER,
			x - BORDER_TILE_SIZE, y + height, depth,
			M_PI/2, true, true);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_CORNER,
			x + width, y + height, depth,
			0, true, true);
}

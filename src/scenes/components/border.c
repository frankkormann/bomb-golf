#include <citro2d.h>
#include "border.h"
#include "../../rendering/spritesheet.h"
#include "../../util/macros.h"

#define BORDER_TILE_SIZE 10

void Border_DrawLight(int x, int y, float depth, int width, int height) {

	for (int cx = x; cx + BORDER_TILE_SIZE < x + width; cx += BORDER_TILE_SIZE) {
		SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT,
				cx, y - BORDER_TILE_SIZE,
				depth, M_PI/2, true, false);
		SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT,
				cx, y + height,
				depth, M_PI/2, false, false);
	}
	SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT,
			x + width - BORDER_TILE_SIZE, y - BORDER_TILE_SIZE,
			depth, M_PI/2, true, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT,
			x + width - BORDER_TILE_SIZE, y + height,
			depth, M_PI/2, false, false);
	for (int cy = y; cy + BORDER_TILE_SIZE < y + height; cy +=BORDER_TILE_SIZE) {
		SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT,
				x - BORDER_TILE_SIZE, cy,
				depth, 0, true, false);
		SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT,
				x + width, cy,
				depth, 0, false, false);
	}
	SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT,
			x - BORDER_TILE_SIZE, y + height - BORDER_TILE_SIZE,
			depth, 0, true, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT,
			x + width, y + height - BORDER_TILE_SIZE,
			depth, 0, false, false);

	SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT_CORNER,
			x - BORDER_TILE_SIZE, y - BORDER_TILE_SIZE,
			depth, 0, false, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT_CORNER,
			x + width, y - BORDER_TILE_SIZE,
			depth, M_PI/2, false, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT_CORNER,
			x - BORDER_TILE_SIZE, y + height,
			depth, M_PI/2, true, true);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_LIGHT_CORNER,
			x + width, y + height,
			depth, 0, true, true);
}

void Border_DrawDark(int x, int y, float depth, int width, int height) {

	for (int cx = x; cx + BORDER_TILE_SIZE < x + width; cx += BORDER_TILE_SIZE) {
		SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK,
				cx, y - BORDER_TILE_SIZE,
				depth, M_PI/2, true, false);
		SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK,
				cx, y + height,
				depth, M_PI/2, false, false);
	}
	SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK,
			x + width - BORDER_TILE_SIZE, y - BORDER_TILE_SIZE,
			depth, M_PI/2, true, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK,
			x + width - BORDER_TILE_SIZE, y + height,
			depth, M_PI/2, false, false);
	for (int cy = y; cy + BORDER_TILE_SIZE < y + height; cy +=BORDER_TILE_SIZE) {
		SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK,
				x - BORDER_TILE_SIZE, cy,
				depth, 0, true, false);
		SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK,
				x + width, cy,
				depth, 0, false, false);
	}
	SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK,
			x - BORDER_TILE_SIZE, y + height - BORDER_TILE_SIZE,
			depth, 0, true, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK,
			x + width, y + height - BORDER_TILE_SIZE,
			depth, 0, false, false);

	SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK_CORNER,
			x - BORDER_TILE_SIZE, y - BORDER_TILE_SIZE,
			depth, 0, false, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK_CORNER,
			x + width, y - BORDER_TILE_SIZE,
			depth, M_PI/2, false, false);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK_CORNER,
			x - BORDER_TILE_SIZE, y + height,
			depth, M_PI/2, true, true);
	SpriteSheet_Draw(SPRITE_GUI_BORDER_DARK_CORNER,
			x + width, y + height,
			depth, 0, true, true);
}


/*
 * The glyphs are stored on a spritesheet and drawn as images, instead of using
 * C2D_Text and a font. This is because I couldn't figure out how to export
 * and draw the font without aliasing.
 */

#include <malloc.h>
#include <stdarg.h>
#include <citro2d.h>
#include "text.h"
#include "../../rendering/spritesheet.h"

#define SPACE_WIDTH 5
#define GLYPH_SPACING 1

struct text {
	size_t maxChars;
	char content[];
};

static C2D_SpriteSheet fontSheet;

bool Text_Init() {
	fontSheet = C2D_SpriteSheetLoad("romfs:/gfx/font.t3x");
	return fontSheet;
}

void Text_Exit() {
	C2D_SpriteSheetFree(fontSheet);
}

Text Text_Create(size_t maxChars) {
	if (maxChars == 0) return NULL;

	Text text = malloc(sizeof(struct text) + sizeof(char[maxChars]));
	if (!text) return NULL;

	text->maxChars = maxChars;
	text->content[0] = '\0';

	return text;
}

void Text_Free(Text text) {
	free(text);
}

void Text_SetContent(Text text, char *format, ...) {
	char content[text->maxChars];
	va_list args;
	va_start(args, format);
	vsnprintf(content, text->maxChars, format, args);
	va_end(args);

	size_t i = 0;
	while (i < text->maxChars - 1) {
		text->content[i] = content[i];
		i++;
	}
	text->content[i] = '\0';
}

// Returns the drawn width
float drawGlyph(int index, float x, float y, float depth, u32 color, int size) {
	C2D_Image img = C2D_SpriteSheetGetImage(fontSheet, index);
	C2D_ImageTint tint;
	C2D_PlainImageTint(&tint, color, 1);
	float width = img.subtex->width * size;
	C2D_DrawImage(img, &(C2D_DrawParams) {
		.pos = {
			x,
			y,
			width,
			img.subtex->height * size
		},
		.center = { 0, 0 },
		.depth = depth,
		.angle = 0
	}, &tint);
	return width;
}

void Text_Draw(Text text, float x, float y, float depth, u32 color, int size) {
	float cx = x;
	float cy = y;
	int glyphIndex = -1;
	for (size_t i = 0; i < text->maxChars && text->content[i] != '\0'; i++) {
		if (text->content[i] >= 0x21 && text->content[i] <= 0x7E) {
			glyphIndex = text->content[i] - 0x21;
		} else {
			switch (text->content[i]) {
				case '\n':
					cx = x;
					cy += TEXT_LINE_HEIGHT * size;
					break;
				case ' ':
					cx += (SPACE_WIDTH + GLYPH_SPACING) * size;
					break;
				case TEXT_KEY_A:
					glyphIndex = 94;
					break;
				case TEXT_KEY_B:
					glyphIndex = 95;
					break;
				case TEXT_KEY_X:
					glyphIndex = 96;
					break;
				case TEXT_KEY_Y:
					glyphIndex = 97;
					break;
				case TEXT_KEY_L:
					glyphIndex = 98;
					break;
				case TEXT_KEY_R:
					glyphIndex = 99;
					break;
				case TEXT_KEY_DPAD:
					glyphIndex = 100;
					break;
				case TEXT_KEY_DUP:
					glyphIndex = 101;
					break;
				case TEXT_KEY_DDOWN:
					glyphIndex = 102;
					break;
				case TEXT_KEY_DLEFT:
					glyphIndex = 103;
					break;
				case TEXT_KEY_DRIGHT:
					glyphIndex = 104;
					break;
			}

		}
		if (glyphIndex >= 0) {
			cx += drawGlyph(glyphIndex, cx, cy, depth, color, size)
					+ (GLYPH_SPACING * size);
			glyphIndex = -1;
		}
	}
}

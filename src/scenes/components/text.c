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

typedef enum {
	TOP_LEFT,
	BOTTOM_RIGHT
} GlyphPosition;

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

int getGlyphIndex(char c) {
	if (c >= 0x21 && c <= 0x7E) {
		return c - 0x21;
	} else {
		switch (c) {
			case TEXT_KEY_A:
				return 94;
			case TEXT_KEY_B:
				return 95;
			case TEXT_KEY_X:
				return 96;
			case TEXT_KEY_Y:
				return 97;
			case TEXT_KEY_L:
				return 98;
			case TEXT_KEY_R:
				return 99;
			case TEXT_KEY_DPAD:
				return 100;
			case TEXT_KEY_DUP:
				return 101;
			case TEXT_KEY_DDOWN:
				return 102;
			case TEXT_KEY_DLEFT:
				return 103;
			case TEXT_KEY_DRIGHT:
				return 104;
			default:
				return -1;
		}
	}
}

float nextLineWidth(char *str, int size) {
	float width = 0;
	for (char *c = str; *c != '\n' && *c != '\0'; c++) {
		width += GLYPH_SPACING * size;
		if (*c == ' ') {
			width += SPACE_WIDTH * size;
		} else {
			int index = getGlyphIndex(*c);
			C2D_Image img = C2D_SpriteSheetGetImage(fontSheet, index);
			width += img.subtex->width * size;
		}
	}
	return width;
}

// Returns the drawn width
float drawGlyph(int index, float x, float y, float depth, u32 color, int size,
		GlyphPosition pos) {
	C2D_Image img = C2D_SpriteSheetGetImage(fontSheet, index);
	C2D_ImageTint tint;
	C2D_PlainImageTint(&tint, color, 1);
	float width = img.subtex->width * size;
	float height = img.subtex->height * size;
	C2D_DrawImage(img, &(C2D_DrawParams) {
		.pos = {
			pos == TOP_LEFT ? x : x - width,
			pos == TOP_LEFT ? y : y - height,
			width,
			height
		},
		.center = { 0, 0 },
		.depth = depth,
		.angle = 0
	}, &tint);
	return width;
}

void Text_Draw(Text text, float x, float y, float depth, u32 color, int size,
		Text_DrawFlags flags) {
	if (flags == TEXT_RIGHT) {
		x -= nextLineWidth(text->content, size);
	} else if (flags == TEXT_CENTERED) {
		x -= nextLineWidth(text->content, size) / 2;
	}
	float cx = x;
	float cy = y;
	int glyphIndex = -1;
	for (char *c = text->content; *c != '\0'; c++) {
		switch (*c) {
			case '\n':
				cx = x;
				cy += TEXT_LINE_HEIGHT * size;
				break;
			case ' ':
				cx += (SPACE_WIDTH + GLYPH_SPACING) * size;
				break;
			default:
				glyphIndex = getGlyphIndex(*c);
				break;
		}
		if (glyphIndex >= 0) {
			cx += drawGlyph(glyphIndex, cx, cy, depth, color, size,
					TOP_LEFT);
			cx += (GLYPH_SPACING * size);
			glyphIndex = -1;
		}
	}
}

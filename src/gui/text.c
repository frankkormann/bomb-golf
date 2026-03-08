#include <malloc.h>
#include <stdarg.h>
#include <citro2d.h>
#include "text.h"

Text Text_Create(size_t maxChars, C2D_Font font) {
	Text text = malloc(sizeof(struct text));
	if (!text) return NULL;

	// Assumes 1 char -> at most 1 glyph
	text->buf = C2D_TextBufNew(maxChars);
	if (!text->buf) {
		free(text);
		return NULL;
	}

	text->font = font;
	text->maxChars = maxChars;

	C2D_TextParse(&text->text, text->buf, "");

	return text;
}

void Text_Free(Text text) {
	C2D_TextBufDelete(text->buf);
	free(text);
}

char* Text_SetContent(Text text, char *format, ...) {
	char content[text->maxChars];
	va_list args;
	va_start(args, format);
	vsnprintf(content, text->maxChars, format, args);
	va_end(args);

	C2D_TextBufClear(text->buf);
	char *lastParsed = C2D_TextParse(&text->text, text->buf, content);
	C2D_TextOptimize(&text->text);

	return lastParsed;
}

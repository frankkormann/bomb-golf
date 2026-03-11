/*
 * Text that can easily be changed. Handles coordination of C2D_Text with a
 * C2D_TextBuf.
 *
 * To draw, use C2D_DrawText.
 *
 * For static text, it's more efficient to have one C2D_TextBuf provide text
 * for multiple C2D_Text objects, so you should not use this component.
 */

#ifndef TEXT_H
#define TEXT_H

typedef struct text *Text;

struct text {
	C2D_Text text;
	C2D_TextBuf buf;
	C2D_Font font;
	size_t maxChars;
};

/*
 * Creates a Text object that can hold at most maxGlyphs glyphs. Displays
 * content using font. If font is NULL, a system font is used.
 *
 * Returns the Text or NULL on failure.
 */
Text Text_Create(size_t maxChars, C2D_Font font);

void Text_Free(Text text);

/*
 * Uses printf to obtain a string using the given format and arguments, then
 * sets text to display that string.
 *
 * Returns a pointer to the last character which was successfully processed.
 */
char* Text_SetContent(Text text, char *format, ...);

#endif

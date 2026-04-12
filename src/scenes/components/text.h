/*
 * Text using the game's custom font.
 */

#ifndef TEXT_H
#define TEXT_H

#define TEXT_KEY_A '\200'
#define TEXT_KEY_B '\201'
#define TEXT_KEY_X '\202'
#define TEXT_KEY_Y '\203'
#define TEXT_KEY_L '\204'
#define TEXT_KEY_R '\205'
#define TEXT_KEY_DPAD '\206'
#define TEXT_KEY_DUP '\207'
#define TEXT_KEY_DDOWN '\210'
#define TEXT_KEY_DLEFT '\211'
#define TEXT_KEY_DRIGHT '\212'

#define TEXT_LINE_HEIGHT 20

typedef struct text *Text;

/*
 * Loads the custom font. Must be called before any Texts are created.
 *
 * Returns false on failure.
 */
bool Text_Init();

void Text_Exit();

/*
 * Creates a Text object that can hold at most maxChars characters, including
 * the null terminator.
 *
 * Returns the Text or NULL on failure.
 */
Text Text_Create(size_t maxChars);

void Text_Free(Text text);

/*
 * Uses printf to obtain a string using the given format and arguments, then
 * sets text to display that string.
 *
 * If text wasn't initialized with enough space, only part of the string will
 * be displayed.
 */
void Text_SetContent(Text text, char *format, ...);

/*
 * Draws text left-justified. (x, y) is the top-left corner.
 */
void Text_Draw(Text text, float x, float y, float depth, u32 color, int size);

/*
 * Draws text right-justified. (x, y) is the bottom-right corner.
 */
void Text_DrawRight(Text text, float x, float y, float depth, u32 color, int size);

#endif

/*
 * Button which responds to touchscreen input.
 */

#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>
#include "../../util/dispatcher.h"
#include "../../rendering/spritesheet.h"

typedef struct button *Button;

/*
 * Creates a button with its top-left corner at (x, y). The lifetime of onTouch
 * must be at least as long as as the lifetime of the returned button.
 *
 * Returns NULL if an error occurs.
 */
Button Button_Create(float x, float y, SpriteSheet_Sprite icon, void (*onTouch)(void));

void Button_Free(Button button);

/*
 * Registers button receive touch input events from touchDispatcher.
 *
 * Priority should be higher than any components drawn under button.
 * The parameter for dispatched events is ignored.
 *
 * Returns false if button could not be registered.
 */
bool Button_RegisterForTouchEvents(Button button, Dispatcher touchDispatcher,
		int priority);

/*
 * Use this if you are freeing button without freeing touchDispatcher.
 *
 * touchDispatcher should be the same Dispatcher passed to
 * Button_RegisterForTouchEvents.
 */
void Button_RemoveFromTouchDispatcher(Button button, Dispatcher touchDispatcher);

/*
 * Draws button to the current render target at depth, using the position which
 * button was created with.
 */
void Button_Draw(Button button, float depth);

#endif

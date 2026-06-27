/*
 * Button which responds to touchscreen and/or key input.
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
 * If the button will not be registered for key events, keybind can be anything.
 *
 * Returns NULL if an error occurs.
 */
Button Button_Create(float x, float y, SpriteSheet_Sprite icon, int keybind,
		void *onTouchParam, void (*onTouch)(void* param));

void Button_Free(Button button);

/*
 * Registers button receive touch input events from touchDispatcher.
 * Priority should be higher than any components drawn under button.
 *
 * Returns false if button could not be registered.
 *
 * See also: Button_RegisterForKeyEvents
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
 * Registers button receive touch input events from touchDispatcher.
 * Priority should be higher than any components drawn under button.
 *
 * Returns false if button could not be registered.
 *
 * See also: Button_RegisterForTouchEvents
 */
bool Button_RegisterForKeyEvents(Button button, Dispatcher keyDispatcher,
		int priority);

/*
 * Use this if you are freeing button without freeing keyDispatcher.
 *
 * keyDispatcher should be the same Dispatcher passed to
 * Button_RegisterForKeyEvents.
 */
void Button_RemoveFromTouchDispatcher(Button button, Dispatcher keyDispatcher);

/*
 * Draws button to the current render target at depth, using the position which
 * button was created with.
 */
void Button_Draw(Button button, float depth);

/*
 * While disabled, button will not be drawn and will not response to touch
 * events. Buttons are enabled by default.
 */
void Button_Disable(Button button);

/*
 * Reverses Button_Disable. Buttons are enabled by default.
 */
void Button_Enable(Button button);

#endif

/*
 * Two buttons which represent a binary state; when one is selected, the other
 * is unselected.
 */

#ifndef TOGGLE_H
#define TOGGLE_H

#include <stdbool.h>
#include "../../util/dispatcher.h"

typedef struct toggle *Toggle;

typedef struct {
	char *label;
	int value;
} Toggle_Button;

/*
 * Creates a toggle with the chosen buttons. Each label should be relatively
 * small so it doesn't overflow the button.
 *
 * The buttons are rendered in a horizontal row. (x, y) is the top-left
 * position of the first button in the row. Buttons are drawn from left to
 * right in the order they are in buttons.
 *
 * The button whose position in buttons matches initIndex will be initially
 * selected.
 *
 * Returns NULL if an error occurs.
 */
Toggle Toggle_Create(float x, float y, int numButtons, int initIndex,
		Toggle_Button buttons[]);

void Toggle_Free(Toggle toggle);

/*
 * Returns the value for toggle's current state as specified in its creation.
 */
int Toggle_GetValue(Toggle toggle);

/*
 * Registers toggle to receive touch input events from touchDispatcher.
 * Priority should be higher than any components drawn under button.
 *
 * Returns false if toggle could not be registered.
 */
bool Toggle_RegisterForTouchEvents(Toggle toggle, Dispatcher touchDispatcher,
		int priority);

/*
 * Use this if you are freeing toggle without freeing touchDispatcher.
 *
 * touchDispatcher should be the same Dispatcher passed to
 * Toggle_RegisterForTouchEvents.
 */
void Toggle_RemoveFromTouchDispatcher(Toggle toggle, Dispatcher touchDispatcher);

/*
 * Uses the position toggle was created with.
 */
void Toggle_Draw(Toggle toggle, float depth);

#endif

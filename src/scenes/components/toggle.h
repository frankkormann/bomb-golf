/*
 * Two buttons which represent a binary state; when one is selected, the other
 * is unselected.
 */

#ifndef TOGGLE_H
#define TOGGLE_H

#include <stdbool.h>
#include "../../util/dispatcher.h"

typedef struct toggle *Toggle;

/*
 * Creates a toggle with the chosen labels and values for its two states. Each
 * label should be relatively small so it doesn't overflow the button.
 *
 * Returns NULL if an error occurs.
 */
Toggle Toggle_Create(float x, float y, const char *leftLabel,
		const char *rightLabel, int leftVal, int rightVal);

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

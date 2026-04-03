/*
 * Represents an individual level that can be played or edited. Displays the
 * level number and contains a button to play the level and a button to edit it.
 */

#ifndef LEVELCARD_H
#define LEVELCARD_H

#include <stdbool.h>
#include "../../util/dispatcher.h"

typedef struct levelcard *LevelCard;

/*
 * When the card is pressed anywhere except the Play or Edit buttons, onPress
 * is called.
 *
 * Returns NULL on failure.
 */
LevelCard LevelCard_Create(float x, float y, int levelNum,
		void (*onPress)(int levelNum));

void LevelCard_Free(LevelCard levelCard);

/*
 * Registers levelCard to receive touch input events from touchDispatcher.
 *
 * Returns false if levelCard could not be registered.
 */
bool LevelCard_RegisterForTouchEvents(LevelCard levelCard,
		Dispatcher touchDispatcher, int priority);

/*
 * Use this if you are freeing levelCard without freeing touchDispatcher.
 *
 * touchDispatcher should be the same Dispatcher passed to
 * LevelCard_RegisterForTouchEvents.
 */
void LevelCard_RemoveFromTouchDispatcher(LevelCard levelCard,
		Dispatcher touchDispatcher);

void LevelCard_Draw(LevelCard levelCard, float depth);

#endif

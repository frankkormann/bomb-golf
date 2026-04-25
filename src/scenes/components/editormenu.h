/*
 * Collapsible menu with buttons to save and quit, quit without saving, edit
 * name or increase/decrease par.
 */

#ifndef EDITORMENU_H
#define EDITORMENU_H

#include <stdbool.h>
#include "../../util/dispatcher.h"

/*
 * Creates the menu with the specified callbacks for its Buttons.
 *
 * Returns false if an error occurs.
 */
bool EditorMenu_Init(
		void (*editName)(void *ignored),
		void (*saveExit)(void *ignored),
		void (*exitNoSave)(void *ignored),
		void (*changePar)(int change)
	);
void EditorMenu_Exit();

/*
 * Registers the menu to receive touch input events from touchDispatcher.
 *
 * Priority should be higher than any components drawn under the menu.
 *
 * Returns false if the menu could not be registered.
 */
bool EditorMenu_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority);

/*
 * Use this if you are exiting the menu without freeing touchDispatcher.
 *
 * touchDispatcher should be the same Dispatcher passed to
 * EditorMenu_RegisterForTouchEvents.
 */
void EditorMenu_RemoveFromTouchDispatcher(Dispatcher touchDispatcher);

/*
 * If the menu is collapsed, draws a button to expand it. Otherwise draws the
 * full menu.
 */
void EditorMenu_Draw(float depth);

#endif

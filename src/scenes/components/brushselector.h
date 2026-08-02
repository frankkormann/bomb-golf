/*
 * Menu that allows the user to select from different brush types. A brush
 * controls what happens the user touches a tile in the level editor.
 */

#ifndef BRUSHSELECTOR_H
#define BRUSHSELECTOR_H

#include <stdbool.h>
#include "../../util/dispatcher.h"

typedef enum {
	BRUSH_PENCIL,
	BRUSH_RECTANGLE,
	BRUSH_BALL_POS,
	BRUSH_HOLE_POS,
	BRUSH_OBSTACLE_ADD,
	BRUSH_OBSTACLE_DEL,
	BRUSH_OBSTACLE_EDIT,
	BRUSH_OBSTACLE_MOVE,
	BRUSH_OBSTACLE_DUPE,

	NUM_BRUSHES
} BrushSelector_Brush;

/*
 * Intializes the brush selector.
 *
 * Returns false if an error occurs.
 */
bool BrushSelector_Init(BrushSelector_Brush defaultBrush);

void BrushSelector_Exit();

/*
 * Registers the brush selector to receive touch input events from
 * touchDispatcher.
 *
 * Priority should be higher than any components drawn under it.
 *
 * Returns false if the brush selector could not be registered.
 */
bool BrushSelector_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority);

/*
 * Use this if you are exiting the brush selector without freeing
 * touchDispatcher.
 *
 * touchDispatcher should be the same Dispatcher passed to
 * BrushSelector_RegisterForTouchEvents.
 */
void BrushSelector_RemoveFromTouchDispatcher(Dispatcher touchDispatcher);

/*
 * Returns the currently selected brush type.
 */
BrushSelector_Brush BrushSelector_GetBrush();

void BrushSelector_SetBrush(BrushSelector_Brush brush);

void BrushSelector_Draw(float depth);

#endif

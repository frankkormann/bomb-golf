/*
 * Menu that allows the creation and alteration of Obstacle_Data objects.
 */

#ifndef OBSTACLEEDITOR_H
#define OBSTACLEEDITOR_H

#include <stdbool.h>
#include "../../environment/obstacle.h"
#include "../../util/dispatcher.h"

/*
 * Returns false if an error occurs.
 */
bool ObstacleEditor_Init();

void ObstacleEditor_Exit();

/*
 * The editor will dismiss itself when done. toEdit is altered in-place.
 */
void ObstacleEditor_Show(Obstacle_Data *toEdit);

/*
 * Registers the editor to receive touch input events from touchDispatcher.
 *
 * Priority should be higher than any components drawn under the editor.
 *
 * Returns false if the editor could not be registered.
 */
bool ObstacleEditor_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority);

/*
 * Use this if you are exiting the editor without freeing touchDispatcher.
 *
 * touchDispatcher should be the same Dispatcher passed to
 * ObstacleEditor_RegisterForTouchEvents.
 */
void ObstacleEditor_RemoveFromTouchDispatcher(Dispatcher touchDispatcher);

void ObstacleEditor_Draw(float depth);

#endif

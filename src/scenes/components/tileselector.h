/*
 * Menu that allows the user to select a Tile and maintain a bar of recently-
 * used Tiles. Should be drawn on the bottom screen.
 */

#ifndef TILESELECTOR_H
#define TILESELECTOR_H

#include <stdbool.h>
#include "../../tile.h"
#include "../../util/dispatcher.h"

/*
 * Intializes the tile selector.
 *
 * Returns false if an error occurs.
 */
bool TileSelector_Init(Tile defaultTile);

void TileSelector_Exit();

/*
 * Handles input events and updates the currently selected tile.
 *
 * An alternative to using a dispatcher for input events.
 */
void TileSelector_Update();

/*
 * Registers tile selector to receive touch input events from touchDispatcher.
 *
 * Priority should be higher than any components drawn under the tile selector.
 * The parameter for dispatched events is ignored.
 *
 * An alternative to using TileSelector_Update.
 */
void TileSelector_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority);

/*
 * Call this once per graphics frame.
 */
void TileSelector_UpdateGraphics();

/*
 * Returns the currently selected Tile.
 */
Tile TileSelector_GetTile();

/*
 * Returns true if the tile selector is in full menu mode. It should be drawn
 * on top of all other components. If not using a dispatcher for touch events,
 * the GUI manager should pause touch handling on other components.
 */
bool TileSelector_IsShowingPopup();

/*
 * In recently-used bar mode, draws the bar across the top of the screen.
 */
void TileSelector_Draw(float depth);

#endif

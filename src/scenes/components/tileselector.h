/*
 * Menu that allows the user to select a Tile and maintain a bar of recently-
 * used Tiles. Should be drawn on the bottom screen.
 */

#ifndef TILESELECTOR_H
#define TILESELECTOR_H

#include <stdbool.h>
#include "../../tile.h"
#include "../../util/dispatcher.h"

#define TILE_SELECTOR_HEIGHT TILE_SIZE

/*
 * Intializes the tile selector.
 *
 * Returns false if an error occurs.
 */
bool TileSelector_Init(Tile defaultTile);

void TileSelector_Exit();

/*
 * Registers the tile selector to receive touch input events from
 * touchDispatcher.
 *
 * Priority should be higher than any components drawn under the tile selector.
 *
 * Returns false if the tile selector could not be registered.
 */
bool TileSelector_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority);

/*
 * Use this if you are exiting the tile selector without freeing
 * touchDispatcher.
 *
 * touchDispatcher should be the same Dispatcher passed to
 * TileSelector_RegisterForTouchEvents.
 */
void TileSelector_RemoveFromTouchDispatcher(Dispatcher touchDispatcher);

/*
 * Updates the Backgrounds used by the tile selector.
 */
void TileSelector_UpdateGraphics();

/*
 * Returns the currently selected Tile.
 */
Tile TileSelector_GetTile();

/*
 * In recently-used bar mode, draws the bar across the top of the screen.
 */
void TileSelector_Draw(float depth);

#endif

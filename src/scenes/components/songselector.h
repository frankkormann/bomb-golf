/*
 * Menu that allows a Music_Song to be chosen. User can listen to each song
 * before selecting one.
 */

#ifndef SONGSELECTOR_H
#define SONGSELECTOR_H

#include "../../audio/music.h"
#include "../../util/dispatcher.h"

/*
 * Returns false if an error occurs.
 */
bool SongSelector_Init();

void SongSelector_Exit();

/*
 * Edits toEdit in place. When dismissed, the currently-playing song is set
 * to restore.
 */
void SongSelector_Show(Music_Song *toEdit, Music_Song restore);

/*
 * Registers the menu to receive touch input events from touchDispatcher.
 *
 * Priority should be higher than any components drawn under the editor.
 *
 * Returns false if the editor could not be registered.
 */
bool SongSelector_RegisterForTouchEvents(Dispatcher touchDispatcher, int priority);

/*
 * Use this if you are exiting the menu without freeing touchDispatcher.
 *
 * touchDispatcher should be the same Dispatcher passed to
 * SongSelector_RegisterForTouchEvents.
 */
void SongSelector_RemoveFromTouchDispatcher(Dispatcher touchDispatcher);

void SongSelector_Draw(float depth);

#endif

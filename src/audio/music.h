/*
 * Plays back looping music. Only one song can play at a time.
 */

#ifndef MUSIC_H
#define MUSIC_H

#include <stdbool.h>

// These values are written directly into level files, so be careful when
// reassigning or removing them
typedef enum {
	MUSIC_NONE,
	MUSIC_LEVEL_1,
	MUSIC_LEVEL_2,
	MUSIC_RESULTS,
	MUSIC_EDITOR,
//	MUSIC_SUMMARY,  // Future idea
//	MUSIC_CREDITS,  // Future idea

	NUM_MUSIC_SONGS
} Music_Song;

/*
 * Loads all Music_Songs for playback.
 */
bool Music_Init();

void Music_Exit();

/*
 * Begins playing song. Music will automatically loop. If a song is already
 * playing, replaces it--there is no need to call Music_Stop between songs.
 *
 * Music_Init must be called first, exactly once.
 */
bool Music_Start(Music_Song song);

/*
 * Stops the current song.
 */
void Music_Stop();

#endif

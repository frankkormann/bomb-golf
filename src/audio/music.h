/*
 * Plays back looping music. Only one song can play at a time.
 */

#ifndef MUSIC_H
#define MUSIC_H

#include <stdbool.h>

typedef enum {
	MUSIC_LEVEL,
	MUSIC_RESULTS,
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
 * Exits and frees any resources being used.
 */
void Music_Stop();

#endif

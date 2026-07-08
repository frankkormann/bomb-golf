/*
 * Plays back looping music. Only one song can play at a time.
 */

#ifndef MUSIC_H
#define MUSIC_H

#include <stdbool.h>

/*
 * Begins playing the song at path. Only opus files are supported. Music will
 * automatically loop. If a song is already playing, replaces it--there is no
 * need to call Music_Stop between songs.
 */
bool Music_Start(char *path);

/*
 * Exits and frees any resources being used.
 */
void Music_Stop();

#endif

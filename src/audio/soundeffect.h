/*
 * Plays back sound effects, small audio files. Each audio file is loaded into
 * memory to enable fast repetitive playbacks.
 */

#ifndef SOUNDEFFECT_H
#define SOUNDEFFECT_H

typedef struct soundeffect* SoundEffect;

/*
 * Returns the SoundEffect or NULL if there was an error (for example, the file
 * is too large).
 *
 * Note: Only opus files are supported.
 */
SoundEffect SoundEffect_Create(char *path);

/*
 * Begins sfx. If it's already playing, restarts it.
 */
void SoundEffect_Play(SoundEffect sfx);

/*
 * Ends sfx prematurely.
 */
void SoundEffect_Stop(SoundEffect sfx);

void SoundEffect_Free(SoundEffect sfx);

#endif

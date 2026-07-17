/*
 * Plays back sound effects, small audio files. Each audio file is loaded into
 * memory to enable fast repetitive playbacks.
 */

#ifndef SOUNDEFFECT_H
#define SOUNDEFFECT_H

#include <stdbool.h>

typedef enum {
	SFX_EXPLOSION,
	SFX_BOUNCE,
	SFX_FIREWORK_EXPLOSION,
//	SFX_HOLE_IN_ONE,  // Future idea

	NUM_SOUND_EFFECTS
} SoundEffect;

/*
 * Loads all sound effects into memory. Call this first.
 */
bool SoundEffect_Init();

void SoundEffect_Exit();

/*
 * Begins sfx. If it's already playing, optionally restarts it or does nothing.
 */
void SoundEffect_Play(SoundEffect sfx, bool restart);

/*
 * Ends sfx prematurely.
 */
void SoundEffect_Stop(SoundEffect sfx);

#endif

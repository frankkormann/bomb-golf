/*
 * Much of this file would not be possible without the work done by thejsa and
 * mtheall in providing example code for decoding/playing back audio.
 */

#include <stdbool.h>
#include <malloc.h>
#include <3ds.h>
#include <opusfile.h>
#include "soundeffect.h"

#define SAMPLE_RATE 48000
#define CHNS_PER_SAMPLE 2

#define MIN_CHN 1
#define MAX_CHN 23

typedef struct {
	ndspWaveBuf waveBuf;
	s16 *audioBuffer;
	int chn;
} SfxObj;

static char *sfxPaths[NUM_SOUND_EFFECTS] = {
	"romfs:sfx/explosion.opus",
	"romfs:sfx/bounce.opus",
	"romfs:sfx/explosion.opus"  //TODO Make FIREWORK_EXPLOSION different
	                            // from EXPLOSION--more crinkly
};

static SfxObj sfxObjs[NUM_SOUND_EFFECTS];

static void fillBuffer(s16 *bufStart, s16 *bufEnd, OggOpusFile *opusFile) {
	int readSamples = 0;
	s16 *buf = bufStart;
	do {
		buf += readSamples * CHNS_PER_SAMPLE;
		readSamples = op_read_stereo(opusFile, buf, bufEnd - buf);
	} while (readSamples > 0);
}

bool fillSfxObj(SfxObj *sfx, char *path, int chn) {
	OggOpusFile *opusFile = op_open_file(path, NULL);
	if (!opusFile) goto f_opusFile;

	size_t totalSamples = op_pcm_total(opusFile, -1);
	sfx->audioBuffer = linearAlloc(sizeof(sfx->audioBuffer) * totalSamples
			* CHNS_PER_SAMPLE);
	if (!sfx->audioBuffer) goto f_audioBuffer;

	sfx->waveBuf.data_vaddr = sfx->audioBuffer;
	fillBuffer(sfx->audioBuffer,
			sfx->audioBuffer + (totalSamples * CHNS_PER_SAMPLE),
			opusFile);
	sfx->waveBuf.nsamples = totalSamples;

	sfx->chn = chn;

	op_free(opusFile);
	return true;

f_audioBuffer:
	op_free(opusFile);
f_opusFile:
	return false;
}

void freeSfxObj(SfxObj *sfx) {
	if (sfx->audioBuffer) linearFree(sfx->audioBuffer);
}

bool SoundEffect_Init() {
	for (SoundEffect i = 0; i < NUM_SOUND_EFFECTS; i++) {
		if (!fillSfxObj(&sfxObjs[i], sfxPaths[i], i + MIN_CHN)) {
			for (SoundEffect j = 0; j < i; j++) freeSfxObj(&sfxObjs[j]);
			return false;
		}
	}
	return true;
}

void SoundEffect_Exit() {
	for (SoundEffect i = 0; i < NUM_SOUND_EFFECTS; i++) {
		freeSfxObj(&sfxObjs[i]);
	}
}

void SoundEffect_Play(SoundEffect sfxIndex, bool restart) {
	SfxObj *sfx = &sfxObjs[sfxIndex];
	if (!restart && ndspChnIsPlaying(sfx->chn)) return;

	ndspChnReset(sfx->chn);
	ndspChnSetInterp(sfx->chn, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(sfx->chn, SAMPLE_RATE);
	ndspChnSetFormat(sfx->chn, NDSP_FORMAT_STEREO_PCM16);

	ndspChnWaveBufAdd(sfx->chn, &sfx->waveBuf);
	DSP_FlushDataCache(sfx->waveBuf.data_pcm16,
			sizeof(sfx->audioBuffer) * sfx->waveBuf.nsamples
					* CHNS_PER_SAMPLE);
}

void SoundEffect_Stop(SoundEffect sfxIndex) {
	ndspChnReset(sfxObjs[sfxIndex].chn);
}

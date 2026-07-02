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

struct soundeffect {
	ndspWaveBuf waveBuf;
	s16 *audioBuffer;
	int chn;
};

static bool chnInUse[MAX_CHN - MIN_CHN + 1];

static int reserveChannel() {
	for (int i = MIN_CHN; i <= MAX_CHN; i++) {
		if (chnInUse[i - MIN_CHN]) continue;
		chnInUse[i - MIN_CHN] = true;
		return i;
	}
	return -1;
}

static void freeChannel(int chn) {
	chnInUse[chn - MIN_CHN] = false;
}

static void fillBuffer(s16 *bufStart, s16 *bufEnd, OggOpusFile *opusFile) {
	int readSamples = 0;
	s16 *buf = bufStart;
	do {
		buf += readSamples * CHNS_PER_SAMPLE;
		readSamples = op_read_stereo(opusFile, buf, bufEnd - buf);
	} while (readSamples > 0);
}

SoundEffect SoundEffect_Create(char *path) {
	SoundEffect sfx = malloc(sizeof(*sfx));
	if (!sfx) goto f_sfx;

	sfx->chn = reserveChannel();
	if (sfx->chn < 0) goto f_chn;

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

	op_free(opusFile);

	return sfx;

f_audioBuffer:
	op_free(opusFile);
f_opusFile:
f_chn:
	free(sfx);
f_sfx:
	return NULL;
}

void SoundEffect_Play(SoundEffect sfx) {
	ndspChnReset(sfx->chn);
	ndspChnSetInterp(sfx->chn, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(sfx->chn, SAMPLE_RATE);
	ndspChnSetFormat(sfx->chn, NDSP_FORMAT_STEREO_PCM16);

	ndspChnWaveBufAdd(sfx->chn, &sfx->waveBuf);
	DSP_FlushDataCache(sfx->waveBuf.data_pcm16,
			sizeof(sfx->audioBuffer) * sfx->waveBuf.nsamples
					* CHNS_PER_SAMPLE);
}

void SoundEffect_Stop(SoundEffect sfx) {
	ndspChnReset(sfx->chn);
}

void SoundEffect_Free(SoundEffect sfx) {
	linearFree(sfx->audioBuffer);
	freeChannel(sfx->chn);
	free(sfx);
}

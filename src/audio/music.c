/*
 * Much of this file would not be possible without the work done by thejsa and
 * mtheall in providing example code for decoding/playing back audio.
 */

#include <stdbool.h>
#include <string.h>
#include <3ds.h>
#include <opusfile.h>
#include "music.h"
#include "../util/macros.h"

#define SAMPLE_RATE 48000
#define SAMPLES_PER_BUF (SAMPLE_RATE * 120 / 1000)  /* 120 ms buffer */
#define CHNS_PER_SAMPLE 2

#define SAMPLES_PER_WAVEBUF (SAMPLES_PER_BUF * CHNS_PER_SAMPLE)
#define NUM_WAVEBUFS 3  /* Triple-buffered */

#define THREAD_STACK_SIZE 0x8000

#define MUSIC_CHN 0

#define DECIBELS(x) (256 * x)

static char *songPaths[NUM_MUSIC_SONGS] = {
	"romfs:music/Bit Shift.opus",
	"romfs:music/Itty Bitty 8 Bit (Beginning).opus",
	"romfs:music/Deliberate Thought.opus"
};
static int songGains[NUM_MUSIC_SONGS] = {
	DECIBELS(-3),
	DECIBELS(-1),
	DECIBELS(4)
};

static OggOpusFile *opusFiles[NUM_MUSIC_SONGS];
static Music_Song song;

static ndspWaveBuf waveBufs[NUM_WAVEBUFS];
static s16 *audioBuffer;

static LightEvent event;
static bool eventInitd = false;
static volatile bool playing;
static Thread thread;

bool Music_Init() {
	for (Music_Song i = 0; i < NUM_MUSIC_SONGS; i++) {
		opusFiles[i] = op_open_file(songPaths[i], NULL);
		if (!opusFiles[i]) {
			for (Music_Song j = 0; j < i; j++) op_free(opusFiles[j]);
			return false;
		}
		op_set_gain_offset(opusFiles[i], OP_ABSOLUTE_GAIN, songGains[i]);
	}
	return true;
}

void Music_Exit() {
	for (Music_Song i = 0; i < NUM_MUSIC_SONGS; i++) {
		op_free(opusFiles[i]);
	}
}

static void audioCallback() {
	if (!playing) return;
	LightEvent_Signal(&event);
}

static int fillBuffer(s16 *bufStart, s16 *bufEnd) {
	int totalSamples = 0, readSamples = 0;
	s16 *buf = bufStart;
	do {
		readSamples = op_read_stereo(opusFiles[song], buf, bufEnd - buf);
		buf += readSamples * CHNS_PER_SAMPLE;
		totalSamples += readSamples;
	} while (readSamples > 0 && buf < bufEnd);
	return totalSamples;
}

static void audioThread() {
	while (playing) {
		for (size_t i = 0; i < NUM_WAVEBUFS; i++) {
			if (waveBufs[i].status != NDSP_WBUF_DONE) continue;

			int samples = fillBuffer(waveBufs[i].data_pcm16,
					waveBufs[i].data_pcm16 +SAMPLES_PER_WAVEBUF);
			if (samples == 0) {
				op_raw_seek(opusFiles[song], 0);
			}
			waveBufs[i].nsamples = samples;
			ndspChnWaveBufAdd(MUSIC_CHN, &waveBufs[i]);
			DSP_FlushDataCache(waveBufs[i].data_pcm16,
					sizeof(audioBuffer) * samples
						* CHNS_PER_SAMPLE);
		}
		LightEvent_Wait(&event);
	}
}

bool Music_Start(Music_Song argSong) {
	if (!eventInitd) {
		LightEvent_Init(&event, RESET_ONESHOT);
		eventInitd = true;
	}
	ndspChnReset(MUSIC_CHN);
	ndspChnSetInterp(MUSIC_CHN, NDSP_INTERP_POLYPHASE);
	ndspChnSetRate(MUSIC_CHN, SAMPLE_RATE);
	ndspChnSetFormat(MUSIC_CHN, NDSP_FORMAT_STEREO_PCM16);

	if (!audioBuffer) {
		audioBuffer = linearAlloc(sizeof(*audioBuffer) * SAMPLES_PER_WAVEBUF
				* NUM_WAVEBUFS);
		if (!audioBuffer) goto f_audioBuffer;
	}
	memset(waveBufs, 0, sizeof(waveBufs));
	s16 *buf = audioBuffer;
	for (size_t i = 0; i < NUM_WAVEBUFS; i++) {
		waveBufs[i].data_vaddr = buf;
		waveBufs[i].status = NDSP_WBUF_DONE;
		buf += SAMPLES_PER_WAVEBUF;
	}

	song = argSong;
	playing = true;
	op_raw_seek(opusFiles[song], 0);
	ndspSetCallback(audioCallback, NULL);

	s32 threadPriority = 0x30;
	svcGetThreadPriority(&threadPriority, CUR_THREAD_HANDLE);
	threadPriority = clamp(threadPriority + 1, 0x18, 0x3F);
	thread = threadCreate(audioThread, NULL, THREAD_STACK_SIZE,
			threadPriority, -1, false);

	return true;

f_audioBuffer:
	return false;
}

void Music_Stop() {
	// Make sure thread is finished
	playing = false;
	LightEvent_Signal(&event);
	threadJoin(thread, UINT64_MAX);

	threadFree(thread);
	if (audioBuffer) {
		linearFree(audioBuffer);
		audioBuffer = NULL;
	}
	ndspChnReset(MUSIC_CHN);
}

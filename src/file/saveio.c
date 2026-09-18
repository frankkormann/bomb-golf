#include <stdio.h>
#include <limits.h>
#include "saveio.h"
#include "savedir.h"

#define SAVEFILE "save.bin"

/*
 * Save file layout:
 *     18 ints for high scores
 *      1 int  for overall high score
 */

#define HISCORE_OFFSET		0
#define TOTAL_HISCORE_OFFSET	(HISCORE_OFFSET + sizeof(int) * 18)

FILE* openSaveFile() {
	char path[32];
	sprintf(path, "%s%s", SaveDir_Root(), SAVEFILE);
	FILE *f = fopen(path, "rb+");
	if (!f) {
		f = fopen(path, "wb");
		if (!f) return NULL;
		fseek(f, HISCORE_OFFSET, SEEK_SET);
		for (int i = 0; i < 18; i++) {
			fwrite((int[]) { INT_MAX }, sizeof(int), 1, f);
		}
		// Have to do this so the results from the fwrites are visible
		// in potential freads
		// For some reason fflush doesn't do it
		fclose(f);
		f = fopen(path, "rb+");
	}
	return f;
}

bool SaveIO_ReadHighScores(int scores[18]) {
	FILE *f = openSaveFile();
	if (!f) return false;

	fseek(f, HISCORE_OFFSET, SEEK_SET);
	if (fread(scores, sizeof(int), 18, f) < 18) {
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

bool SaveIO_UpdateScore(int level, int score, bool *didUpdate) {
	FILE *f = openSaveFile();
	if (!f) return false;

	int prevScore;
	fseek(f, HISCORE_OFFSET + (level * sizeof(int)), SEEK_SET);
	if (fread(&prevScore, sizeof(int), 1, f) < 1) {
		fclose(f);
		return false;
	}
	if (score < prevScore) {
		fseek(f, -sizeof(int), SEEK_CUR);
		if (fwrite(&score, sizeof(int), 1, f) < 1) {
			fclose(f);
			return false;
		}
	}
	if (didUpdate) *didUpdate = score < prevScore;
	fclose(f);
	return true;
}

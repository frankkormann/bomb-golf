#include <stdio.h>
#include <limits.h>
#include <3ds.h>
#include "saveio.h"
#include "savedir.h"
#include "../util/macros.h"

#define SAVEFILE "save.bin"

/*
 * Save file layout:
 *     18 ints  for high scores
 *      1 int   for overall high score
 *      1 u16   for achievements
 *      1 bool  for mirrored setting
 *      1 u8    for explosion controls setting
 *      1 float for game speed
 */

#define HISCORE_OFFSET		0
#define TOTAL_HISCORE_OFFSET	(HISCORE_OFFSET + sizeof(int) * 18)
#define ACHVMT_OFFSET		(TOTAL_HISCORE_OFFSET + sizeof(int))
#define MIRRORED_OFFSET		(ACHVMT_OFFSET + sizeof(u16))
#define EXPLOSION_OFFSET	(MIRRORED_OFFSET + sizeof(bool))
#define SPEED_OFFSET		(EXPLOSION_OFFSET + sizeof(u8))

FILE* openSaveFile() {
	char path[32];
	sprintf(path, "%s%s", SaveDir_Root(), SAVEFILE);
	FILE *f = fopen(path, "rb+");
	if (!f) {
		f = fopen(path, "wb");
		if (!f) return NULL;
		fseek(f, HISCORE_OFFSET, SEEK_SET);
		for (int i = 0; i < 19; i++) {
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

	int prev;
	fseek(f, HISCORE_OFFSET + (level * sizeof(int)), SEEK_SET);
	if (fread(&prev, sizeof(int), 1, f) < 1) {
		fclose(f);
		return false;
	}
	if (score < prev) {
		fseek(f, -sizeof(int), SEEK_CUR);
		if (fwrite(&score, sizeof(int), 1, f) < 1) {
			fclose(f);
			return false;
		}
	}
	if (didUpdate) {
		*didUpdate = prev != INT_MAX && score < prev;
	}
	fclose(f);
	return true;
}

bool SaveIO_ReadOverallHighScore(int *score) {
	FILE *f = openSaveFile();
	if (!f) return false;

	fseek(f, TOTAL_HISCORE_OFFSET, SEEK_SET);
	if (fread(score, sizeof(int), 1, f) < 1) {
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

bool SaveIO_UpdateOverallScore(int score) {
	FILE *f = openSaveFile();
	if (!f) return false;

	int prev;
	fseek(f, TOTAL_HISCORE_OFFSET, SEEK_SET);
	if (fread(&prev, sizeof(int), 1, f) < 1) {
		fclose(f);
		return false;
	}
	if (score < prev) {
		fseek(f, -sizeof(int), SEEK_CUR);
		if (fwrite(&score, sizeof(int), 1, f) < 1) {
			fclose(f);
			return false;
		}
	}
	fclose(f);
	return true;
}

bool SaveIO_ReadAchievements(SaveIO_Achvmt *achievements) {
	FILE *f = openSaveFile();
	if (!f) return false;

	fseek(f, ACHVMT_OFFSET, SEEK_SET);
	if (fread(achievements, min(sizeof(u16), sizeof(SaveIO_Achvmt)), 1, f) < 1) {
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

bool SaveIO_WriteAchievement(SaveIO_Achvmt achievement) {
	FILE *f = openSaveFile();
	if (!f) return false;

	u16 prev = 0;
	fseek(f, ACHVMT_OFFSET, SEEK_SET);
	if (fread(&prev, sizeof(u16), 1, f) == 1) {
		fseek(f, -sizeof(u16), SEEK_CUR);
	}
	prev |= achievement;
	if (fwrite(&prev, sizeof(u16), 1, f) < 1) {
		fclose(f);
		return false;
	}
	fclose(f);
	return true;
}

bool SaveIO_ReadSettings(bool *isMirrored, SaveIO_ExplosionControls *controls,
		float *gameSpeed) {
	FILE *f = openSaveFile();
	if (!f) return false;

	u8 saveControls;

	fseek(f, MIRRORED_OFFSET, SEEK_SET);
	if (fread(isMirrored,  sizeof(bool), 1, f) < 1) goto f_fread;
	if (fread(&saveControls, sizeof(u8), 1, f) < 1) goto f_fread;
	if (fread(gameSpeed,  sizeof(float), 1, f) < 1) goto f_fread;

	*controls = saveControls;

	fclose(f);
	return true;

f_fread:
	fclose(f);
	return false;
}

bool SaveIO_WriteSettings(bool isMirrored, SaveIO_ExplosionControls controls,
		float gameSpeed) {
	FILE *f = openSaveFile();
	if (!f) return false;

	u8 saveControls = controls;

	fseek(f, MIRRORED_OFFSET, SEEK_SET);
	if (fwrite(&isMirrored, sizeof(bool), 1, f) < 1) goto f_fwrite;
	if (fwrite(&saveControls, sizeof(u8), 1, f) < 1) goto f_fwrite;
	if (fwrite(&gameSpeed, sizeof(float), 1, f) < 1) goto f_fwrite;

	fclose(f);
	return true;

f_fwrite:
	fclose(f);
	return false;
}

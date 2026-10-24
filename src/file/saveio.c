#include <stdio.h>
#include <limits.h>
#include <3ds.h>
#include "saveio.h"
#include "savedir.h"
#include "../projectile.h"
#include "../projectiles/bomb.h"
#include "../projectiles/missile.h"
#include "../projectiles/drill.h"
#include "../util/macros.h"

#define SAVEFILE "save.bin"

/*
 * Save file layout:
 *     18 ints  for bomb high scores
 *      1 int   for overall bomb high score
 *     18 ints  for missile high scores
 *      1 int   for overall missile high score
 *     18 ints  for drill high scores
 *      1 int   for overall drill high score
 *      1 u16   for achievements
 *      1 bool  for mirrored setting
 *      1 bool  for touchscreen sensitivity setting
 *      1 float for game speed
 *      1 bool  for big ball setting
 */

#define HISCORE_BOMB_OFF		0
#define TOT_HISCORE_BOMB_OFF		(HISCORE_BOMB_OFF        + sizeof(int) * 18)
#define HISCORE_MISSILE_OFF		(TOT_HISCORE_BOMB_OFF    + sizeof(int))
#define TOT_HISCORE_MISSILE_OFF		(HISCORE_MISSILE_OFF     + sizeof(int) * 18)
#define HISCORE_DRILL_OFF		(TOT_HISCORE_MISSILE_OFF + sizeof(int))
#define TOT_HISCORE_DRILL_OFF		(HISCORE_DRILL_OFF        + sizeof(int) * 18)
#define ACHVMT_OFF			(TOT_HISCORE_DRILL_OFF    + sizeof(int))
#define MIRRORED_OFF			(ACHVMT_OFF              + sizeof(u16))
#define SENSITIVITY_OFF			(MIRRORED_OFF            + sizeof(bool))
#define SPEED_OFF			(SENSITIVITY_OFF         + sizeof(bool))
#define BIGBALL_OFF			(SPEED_OFF               + sizeof(float))

FILE* openSaveFile() {
	char path[32];
	sprintf(path, "%s%s", SaveDir_Root(), SAVEFILE);
	FILE *f = fopen(path, "rb+");
	if (!f) {
		f = fopen(path, "wb");
		if (!f) return NULL;
		// All the high scores are adjacent
		fseek(f, HISCORE_BOMB_OFF, SEEK_SET);
		for (int i = 0; i < 3*18 + 3; i++) {
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

bool SaveIO_ReadHighScores(int bombScores[18], int missileScores[18],
		int drillScores[18]) {
	FILE *f = openSaveFile();
	if (!f) return false;

	fseek(f, HISCORE_BOMB_OFF, SEEK_SET);
	if (fread(bombScores, sizeof(int), 18, f) < 18) {
		fclose(f);
		return false;
	}
	fseek(f, HISCORE_MISSILE_OFF, SEEK_SET);
	if (fread(missileScores, sizeof(int), 18, f) < 18) {
		fclose(f);
		return false;
	}
	fseek(f, HISCORE_DRILL_OFF, SEEK_SET);
	if (fread(drillScores, sizeof(int), 18, f) < 18) {
		fclose(f);
		return false;
	}

	fclose(f);
	return true;
}

bool SaveIO_UpdateScore(int level, int score, Projectile proj, bool *didUpdate) {
	FILE *f = openSaveFile();
	if (!f) return false;

	size_t offset = proj == projectileBomb ? HISCORE_BOMB_OFF
			: proj == projectileMissile ? HISCORE_MISSILE_OFF
			: HISCORE_DRILL_OFF;
	int prev;
	fseek(f, offset + (level * sizeof(int)), SEEK_SET);
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

bool SaveIO_ReadOverallHighScore(int *bombScore, int *missileScore,
		int *drillScore) {
	FILE *f = openSaveFile();
	if (!f) return false;

	fseek(f, TOT_HISCORE_BOMB_OFF, SEEK_SET);
	if (fread(bombScore, sizeof(int), 1, f) < 1) {
		fclose(f);
		return false;
	}
	fseek(f, TOT_HISCORE_MISSILE_OFF, SEEK_SET);
	if (fread(missileScore, sizeof(int), 1, f) < 1) {
		fclose(f);
		return false;
	}
	fseek(f, TOT_HISCORE_DRILL_OFF, SEEK_SET);
	if (fread(drillScore, sizeof(int), 1, f) < 1) {
		fclose(f);
		return false;
	}

	fclose(f);
	return true;
}

bool SaveIO_UpdateOverallScore(int score, Projectile proj) {
	FILE *f = openSaveFile();
	if (!f) return false;

	size_t offset = proj == projectileBomb ? TOT_HISCORE_BOMB_OFF
			: proj == projectileMissile ? TOT_HISCORE_MISSILE_OFF
			: TOT_HISCORE_DRILL_OFF;
	int prev;
	fseek(f, offset, SEEK_SET);
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

	fseek(f, ACHVMT_OFF, SEEK_SET);
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
	fseek(f, ACHVMT_OFF, SEEK_SET);
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

/*
 * If buf is NULL, advances file by size. Otherwise, uses fread to read the
 * next object of size size into buf.
 *
 * Returns true if the object was successfully read or if size is 0.
 */
static bool maybeRead(void *buf, size_t size, FILE *file) {
	if (size == 0) return true;

	if (buf) {
		return fread(buf, size, 1, file);
	} else {
		fseek(file, size, SEEK_CUR);
		return true;
	}
}

bool SaveIO_ReadSettings(bool *isMirrored, bool *isTouchSticky, float *gameSpeed,
		bool *bigBallEnabled) {
	FILE *f = openSaveFile();
	if (!f) return false;

	fseek(f, MIRRORED_OFF, SEEK_SET);
	if (!maybeRead(isMirrored,     sizeof(bool), f)) goto f_maybeRead;
	if (!maybeRead(isTouchSticky,  sizeof(bool), f)) goto f_maybeRead;
	if (!maybeRead(gameSpeed,     sizeof(float), f)) goto f_maybeRead;
	if (!maybeRead(bigBallEnabled, sizeof(bool), f)) goto f_maybeRead;

	fclose(f);
	return true;

f_maybeRead:
	fclose(f);
	return false;
}

bool SaveIO_WriteSettings(bool isMirrored, bool isTouchSticky, float gameSpeed,
		bool bigBallEnabled) {
	FILE *f = openSaveFile();
	if (!f) return false;

	fseek(f, MIRRORED_OFF, SEEK_SET);
	if (fwrite(&isMirrored,     sizeof(bool), 1, f) < 1) goto f_fwrite;
	if (fwrite(&isTouchSticky,  sizeof(bool), 1, f) < 1) goto f_fwrite;
	if (fwrite(&gameSpeed,     sizeof(float), 1, f) < 1) goto f_fwrite;
	if (fwrite(&bigBallEnabled, sizeof(bool), 1, f) < 1) goto f_fwrite;

	fclose(f);
	return true;

f_fwrite:
	fclose(f);
	return false;
}

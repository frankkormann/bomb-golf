/*
 * Read or write general save data. For custom levels, see levelio.h.
 */

#ifndef SAVEIO_H
#define SAVEIO_H

#include <stdbool.h>

typedef enum {
	/* Cleared the course without destroying any obstacles */
	ACHVMT_PEACE		= 1,
	/* Cleared the course and destroyed all obstacles */
	ACHVMT_GENOCIDE		= 2,
	/* Overall score <= -6 */
	ACHVMT_SCORE_GREAT	= 4,
	/* Overall score <=  0 */
	ACHVMT_SCORE_GOOD	= 8,
	/* Overall score <= 10 */
	ACHVMT_SCORE_OK		= 16
} SaveIO_Achvmt;

typedef enum {
	CONTROLS_HOLD,
	CONTROLS_TAPS
} SaveIO_ExplosionControls;

/*
 * Reads the high scores for individual levels into scores. If there is no
 * high score for a level, it is reported as INT_MAX.
 *
 * Returns false if an error occurs.
 */
bool SaveIO_ReadHighScores(int scores[18]);

/*
 * Writes the high score if it is better than the current high score. If
 * didUpdate is not NULL, fills it in with true if the new score was written
 * and false otherwise.
 *
 * Returns false if an error occurs.
 */
bool SaveIO_UpdateScore(int level, int score, bool *didUpdate);

/*
 * Reads the overall high score into score. The overall score is for the entire
 * 18-level campaign. If there is no high score, INT_MAX is reported.
 *
 * Returns false if an error occurs or there is no high score.
 */
bool SaveIO_ReadOverallHighScore(int *score);

/*
 * Writes the overall high score if it is better than the current one. Returns
 * false if an error occurs.
 */
bool SaveIO_UpdateOverallScore(int score);

/*
 * Reads the achieved achievements into achievements as a bit field. Returns
 * false if an error occurs.
 */
bool SaveIO_ReadAchievements(SaveIO_Achvmt *achievements);

/*
 * More than one achievement can be granted by ORing. Returns false if an error
 * occurs.
 */
bool SaveIO_WriteAchievement(SaveIO_Achvmt achievement);

/*
 * Reads settings. If any parameter is NULL, skips that setting.
 *
 * Returns false if an error occurs. In that case, the values of the parameters
 * are undefined.
 */
bool SaveIO_ReadSettings(bool *isMirrored, SaveIO_ExplosionControls *controls,
		float *gameSpeed);

/*
 * Returns false an error occurs.
 */
bool SaveIO_WriteSettings(bool isMirrored, SaveIO_ExplosionControls controls,
		float gameSpeed);

#endif

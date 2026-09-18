/*
 * Read or write general save data. For custom levels, see levelio.h.
 */

#ifndef SAVEIO_H
#define SAVEIO_H

#include <stdbool.h>

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

#endif

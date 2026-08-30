/*
 * Keeps track of numerical statistics across a sequence of levels. Each
 * statistic is zero by default.
 */

#ifndef TRACKER_H
#define TRACKER_H

typedef enum {
	/* Par - strokes */
	TRACKER_LVL1,
	TRACKER_LVL2,
	TRACKER_LVL3,
	TRACKER_LVL4,
	TRACKER_LVL5,
	TRACKER_LVL6,
	TRACKER_LVL7,
	TRACKER_LVL8,
	TRACKER_LVL9,
	TRACKER_LVL10,
	TRACKER_LVL11,
	TRACKER_LVL12,
	TRACKER_LVL13,
	TRACKER_LVL14,
	TRACKER_LVL15,
	TRACKER_LVL16,
	TRACKER_LVL17,
	TRACKER_LVL18,
	/* Number of obstacles destroyed */
	TRACKER_KILLS,

	NUM_TRACKER_STATS
} Tracker_Stat;

int Tracker_Get(Tracker_Stat stat);

/*
 * Reset all statistics to zero.
 */
void Tracker_Clear();

void Tracker_Set(Tracker_Stat stat, int value);

void Tracker_Update(Tracker_Stat stat, int change);

#endif

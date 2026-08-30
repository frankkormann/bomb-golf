#include "tracker.h"

static int stats[NUM_TRACKER_STATS];

int Tracker_Get(Tracker_Stat stat) {
	return stats[stat];
}

void Tracker_Clear() {
	for (Tracker_Stat i = 0; i < NUM_TRACKER_STATS; i++) {
		stats[i] = 0;
	}
}

void Tracker_Set(Tracker_Stat stat, int value) {
	stats[stat] = value;
}

void Tracker_Update(Tracker_Stat stat, int change) {
	stats[stat] += change;
}

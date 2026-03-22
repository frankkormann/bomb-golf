/*
 * Mounts and unmounts save data. Handles differences for CIA or 3dsx build
 * targets.
 */

#ifndef SAVEDATA_H
#define SAVEDATA_H

#include <stdbool.h>

#define SAVEDATA_NUM_LEVELS 18

/*
 * If this is being compiled for a CIA, opens the save data archive. Otherwise
 * does nothing and returns true.
 */
bool SaveData_Mount();

/*
 * If this is being compiled for a CIA, commits save data and closes the
 * archive. Otherwise does nothing.
 */
void SaveData_Unmount();

/*
 * Returns the prefix of the save data, like "romfs" or "sdmc". For a 3dsx this
 * will always be "sdmc".
 */
char* SaveData_GetDeviceName();

#endif

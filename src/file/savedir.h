/*
 * Mounts and unmounts save data. Handles differences for CIA or 3dsx build
 * targets.
 */

#ifndef SAVEDIR_H
#define SAVEDIR_H

#include <stdbool.h>

#define SAVEDIR_NUM_LEVELS 18

/*
 * If this is being compiled for a CIA, opens the save data archive. Otherwise
 * creates a folder on the SD card.
 */
bool SaveDir_Mount();

/*
 * If this is being compiled for a CIA, commits save data and closes the
 * archive. Otherwise does nothing.
 */
void SaveDir_Unmount();

/*
 * Returns the path of the save data's root directory. For a 3dsx this will
 * always be "sdmc:/".
 */
const char* SaveDir_Root();

/*
 * Returns false if an error occured. In this case, nothing is swapped.
 */
bool SaveDir_Swap(const char *path1, const char *path2);

/*
 * Copies the file at src into dest.
 *
 * Returns false if an error occured. In this case, nothing is copied.
 */
bool SaveDir_Copy(const char *dest, const char *src);

#endif

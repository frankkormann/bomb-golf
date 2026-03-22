#include <stdbool.h>
#include <3ds.h>
#include "savedata.h"

#define CIA_DEVICE_NAME "save"

#ifdef _CIA
//https://www.3dbrew.org/wiki/RomFS#Hash_Table_Structure
static int getHashTableLength(int numEntries) {
	int count = numEntries;
	if (numEntries < 3) {
		count = 3;
	 } else if (numEntries < 19) {
		count |= 1;
	 } else {
		while (count % 2 == 0
				|| count % 3 == 0
				|| count % 5 == 0
				|| count % 7 == 0
				|| count % 11 == 0
				|| count % 13 == 0
				|| count % 17 == 0) {
			count++;
		}
	}
	return count;
}
#endif

bool SaveData_Mount() {
	#ifdef _CIA
		Result res = archiveMount(ARCHIVE_SAVEDATA,
				fsMakePath(PATH_EMPTY, ""), CIA_DEVICE_NAME);
		if (R_FAILED(res)) {
			res = FSUSER_FormatSaveData(
					ARCHIVE_SAVEDATA,
					fsMakePath(PATH_EMPTY, ""),
					512,
					0,
					SAVEDATA_NUM_LEVELS,
					getHashTableLength(0),
					getHashTableLength(SAVEDATA_NUM_LEVELS),
					false
				);
			if (R_FAILED(res)) return false;
			res = archiveMount(ARCHIVE_SAVEDATA,
					fsMakePath(PATH_EMPTY, ""), CIA_DEVICE_NAME);
			if (R_FAILED(res)) return false;
		}
		return true;
	#else
		return true;
	#endif
}

void SaveData_Unmount() {
	#ifdef _CIA
		archiveCommitSaveData(CIA_DEVICE_NAME);
		archiveUnmount(CIA_DEVICE_NAME);
	#endif
}

char* SaveData_GetDeviceName() {
	#ifdef _CIA
		return CIA_DEVICE_NAME;
	#else
		return "sdmc";
	#endif
}

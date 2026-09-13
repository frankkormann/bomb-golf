#include <stdbool.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <3ds.h>
#include "savedata.h"

#define CIA_DEVICE_NAME "save"
#define _3DSX_FOLDER "bomb-golf"
#define NUM_FILES (SAVEDATA_NUM_LEVELS + 1)

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
					NUM_FILES,
					getHashTableLength(0),
					getHashTableLength(NUM_FILES),
					false
				);
			if (R_FAILED(res)) return false;
			res = archiveMount(ARCHIVE_SAVEDATA,
					fsMakePath(PATH_EMPTY, ""), CIA_DEVICE_NAME);
			if (R_FAILED(res)) return false;
		}
		return true;
	#else
		DIR *d = opendir(SaveData_Root());
		if (!d) {
			if (mkdir(SaveData_Root(),  0777) != 0) {
				return false;
			}
		} else {
			closedir(d);
		}
		return true;
	#endif
}

void SaveData_Unmount() {
	#ifdef _CIA
		archiveCommitSaveData(CIA_DEVICE_NAME);
		archiveUnmount(CIA_DEVICE_NAME);
	#endif
}

const char* SaveData_Root() {
	#ifdef _CIA
		return CIA_DEVICE_NAME ":/";
	#else
		return "sdmc:/" _3DSX_FOLDER "/";
	#endif
}

bool SaveData_Swap(const char *path1, const char *path2) {
	rename(path1, "temp");
	rename(path2, path1);
	rename("temp", path2);
	return true;
}

bool SaveData_Copy(const char *dest, const char *src) {
	char buf[1024];
	FILE *fdest = fopen(dest, "wb");
	if (!fdest) return false;

	FILE *fsrc = fopen(src, "rb");
	if (!fsrc) {
		fclose(fdest);
		return false;
	}

	size_t num;
	while ((num = fread(buf, sizeof(char), 1024, fsrc)) > 0) {
		if (fwrite(buf, sizeof(char), num, fdest) < num) {
			return false;
		}
	}
	fclose(fdest);
	fclose(fsrc);
	return true;
}

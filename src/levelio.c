#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include <3ds.h>
#include "levelio.h"
#include "tile.h"
#include "savedata.h"
#include "rendering/spritesheet.h"
#include "projectiles/ball.h"

void LevelIO_MakePath(int levelNum, bool inRomfs, char *path) {
	if (inRomfs) {
		sprintf(path, "romfs:/level_%i.bin", levelNum);
	} else {
		sprintf(path, "%s:/level_%i.bin", SaveData_GetDeviceName(),
				levelNum);
	}
}

// Projectile is really a pointer, so convert from int for serialization
static Projectile numToProj(int num) {
	if (num == 0) return projectileBall;
	return NULL;
}

// Projectile is really a pointer, so convert to int for serialization
static int projToNum(Projectile proj) {
	if (proj == projectileBall) return 0;
	return -1;
}

bool LevelIO_Read(const char *path, LevelIO_Hole *hole, LevelIO_Proj *proj,
		Tile (**tiles)[LEVEL_HEIGHT_TILES], int *width, int *par,
		char **name) {
	FILE *data = fopen(path, "rb");
	if (!data) goto f_data;

	if (!fread(width, sizeof(*width), 1, data)) goto f_fread1;
	if (!fread(par,   sizeof(*par),   1, data)) goto f_fread1;

	size_t tilesSize = sizeof(**tiles) * *width / TILE_SIZE;
	*tiles = malloc(tilesSize);
	if (!(*tiles)) goto f_tiles;

	int projNum;
	size_t nameSize;

	if (!fread(&hole->x,      sizeof(hole->x),      1, data)) goto f_fread2;
	if (!fread(&hole->y,      sizeof(hole->y),      1, data)) goto f_fread2;
	if (!fread(&hole->width,  sizeof(hole->width),  1, data)) goto f_fread2;
	if (!fread(&hole->height, sizeof(hole->height), 1, data)) goto f_fread2;
	if (!fread(&proj->startX, sizeof(proj->startX), 1, data)) goto f_fread2;
	if (!fread(&proj->startY, sizeof(proj->startY), 1, data)) goto f_fread2;
	if (!fread(&projNum,      sizeof(projNum),      1, data)) goto f_fread2;
	if (!fread(**tiles,       tilesSize,            1, data)) goto f_fread2;
	if (!fread(&nameSize,     sizeof(nameSize),     1, data)) goto f_fread2;

	*name = malloc(nameSize);
	if (!(*name)) goto f_name;

	if (!fread(*name, nameSize, 1, data)) goto f_fread3;

	proj->type = numToProj(projNum);
	if (!proj->type) goto f_projtype;

	fclose(data);
	return true;

f_projtype:
f_fread3:
	free(*name);
f_name:
f_fread2:
	free(*tiles);
f_tiles:
f_fread1:
	fclose(data);
f_data:
	return false;
}

bool LevelIO_Write(const char *path, LevelIO_Hole hole, LevelIO_Proj proj,
		const Tile (*tiles)[LEVEL_HEIGHT_TILES], int width, int par,
		const char *name) {
	FILE *data = fopen(path, "wb");
	if (!data) goto f_data;

	size_t tilesSize = sizeof(*tiles) * width / TILE_SIZE;
	int projNum = projToNum(proj.type);

	size_t nameSize = (strlen(name) + 1) * sizeof(char);

	if (!fwrite(&width,       sizeof(width),       1, data)) goto f_fwrite;
	if (!fwrite(&par,         sizeof(par),         1, data)) goto f_fwrite;
	if (!fwrite(&hole.x,      sizeof(hole.x),      1, data)) goto f_fwrite;
	if (!fwrite(&hole.y,      sizeof(hole.y),      1, data)) goto f_fwrite;
	if (!fwrite(&hole.width,  sizeof(hole.width),  1, data)) goto f_fwrite;
	if (!fwrite(&hole.height, sizeof(hole.height), 1, data)) goto f_fwrite;
	if (!fwrite(&proj.startX, sizeof(proj.startX), 1, data)) goto f_fwrite;
	if (!fwrite(&proj.startY, sizeof(proj.startY), 1, data)) goto f_fwrite;
	if (!fwrite(&projNum,     sizeof(projNum),     1, data)) goto f_fwrite;
	if (!fwrite(tiles,        tilesSize,           1, data)) goto f_fwrite;
	if (!fwrite(&nameSize,    sizeof(nameSize),    1, data)) goto f_fwrite;
	if (!fwrite(name,         nameSize,            1, data)) goto f_fwrite;

	fclose(data);
	return true;

f_fwrite:
	fclose(data);
f_data:
	return false;
}

#include <malloc.h>
#include <stddef.h>
#include "levelio.h"
#include "rendering/background.h"
#include "projectiles/ball.h"

// Projectile is really a pointer, so convert to int for serialization
static Projectile numToProj(int num) {
	if (num == 0) return projectileBall;
	return NULL;
}

static int projToNum(Projectile proj) {
	if (proj == projectileBall) return 0;
	return -1;
}

bool LevelIO_Read(const char *path, LevelIO_Hole *hole, LevelIO_Proj *proj,
		BG_Tile (**tiles)[LEVEL_HEIGHT_TILES], int *width) {
	FILE *data = fopen(path, "rb");
	if (!data) return false;

	fread(width, sizeof(*width), 1, data);

	size_t tilesSize = sizeof(**tiles) * *width / BG_TILE_SIZE;
	*tiles = malloc(tilesSize);
	if (!tiles) {
		fclose(data);
		return false;
	}

	int projNum;

	fread(&hole->x, sizeof(hole->x), 1, data);
	fread(&hole->y, sizeof(hole->y), 1, data);
	fread(&hole->width, sizeof(hole->width), 1, data);
	fread(&hole->height, sizeof(hole->height), 1, data);
	fread(&proj->startX, sizeof(proj->startX), 1, data);
	fread(&proj->startY, sizeof(proj->startY), 1, data);
	fread(&projNum, sizeof(projNum), 1, data);
	fread(*tiles, tilesSize, 1, data);

	fclose(data);

	if (projNum < 0) {
		free(tiles);
		return false;
	}
	proj->type = numToProj(projNum);
	
	return true;
}

bool LevelIO_Write(const char *path, LevelIO_Hole hole, LevelIO_Proj proj,
		const BG_Tile (*tiles)[LEVEL_HEIGHT_TILES], int width) {
	FILE *data = fopen(path, "wb");
	if (!data) return false;

	size_t tilesSize = sizeof(*tiles) * width / BG_TILE_SIZE;
	int projNum = projToNum(proj.type);

	fwrite(&width, sizeof(width), 1, data);
	fwrite(&hole.x, sizeof(hole.x), 1, data);
	fwrite(&hole.y, sizeof(hole.y), 1, data);
	fwrite(&hole.width, sizeof(hole.width), 1, data);
	fwrite(&hole.height, sizeof(hole.height), 1, data);
	fwrite(&proj.startX, sizeof(proj.startX), 1, data);
	fwrite(&proj.startY, sizeof(proj.startY), 1, data);
	fwrite(&projNum, sizeof(projNum), 1, data);
	fwrite(tiles, tilesSize, 1, data);

	fclose(data);

	return true;
}

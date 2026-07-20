#include <malloc.h>
#include <stddef.h>
#include <string.h>
#include <3ds.h>
#include "levelio.h"
#include "tile.h"
#include "savedata.h"
#include "rendering/spritesheet.h"
#include "projectiles/bomb.h"

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
	if (num == 0) return projectileBomb;
	return NULL;
}

// Projectile is really a pointer, so convert to int for serialization
static int projToNum(Projectile proj) {
	if (proj == projectileBomb) return 0;
	return -1;
}

/*
 * If buf is NULL, advances file by size. Otherwise, uses fread to read the
 * next object of size size into buf.
 *
 * Returns true if the object was successfully read or if size is 0.
 */
bool maybeRead(void *buf, size_t size, FILE *file) {
	if (size == 0) return true;

	if (buf) {
		return fread(buf, size, 1, file);
	} else {
		fseek(file, size, SEEK_CUR);
		return true;
	}
}

static bool readObstacles(LevelIO_Obst **obsts, size_t *argNumObsts, FILE *file) {
	// Make sure we have space to read this
	size_t numObsts;
	if (!maybeRead(&numObsts, sizeof(numObsts), file)) goto f_numObsts;

	if (obsts) {
		*obsts = malloc(sizeof(**obsts) * numObsts);
		if (!(*obsts)) goto f_obsts;
		for (size_t i = 0; i < numObsts; i++) {
			(*obsts)[i].xs = NULL;
			(*obsts)[i].ys = NULL;
		}
	}

	LevelIO_Obst curObst;
	for (size_t i = 0; i < numObsts; i++) {
		if (!maybeRead(&curObst.sprite1,   sizeof(curObst.sprite1),   file))
			goto f_curObst;
		if (!maybeRead(&curObst.sprite2,   sizeof(curObst.sprite2),   file))
			goto f_curObst;
		if (!maybeRead(&curObst.speed,     sizeof(curObst.speed),     file))
			goto f_curObst;
		if (!maybeRead(&curObst.numPoints, sizeof(curObst.numPoints), file))
			goto f_curObst;

		// If we need the points, then allocate them; otherwise set to
		// NULL so maybeRead skips in file
		if (obsts) {
			curObst.xs = malloc(sizeof(*curObst.xs) * curObst.numPoints);
			if (!curObst.xs) goto f_curObst;
			curObst.ys = malloc(sizeof(*curObst.ys) * curObst.numPoints);
			if (!curObst.ys) goto f_curObst;
		} else {
			curObst.xs = NULL;
			curObst.ys = NULL;
		}

		for (int j = 0; j < curObst.numPoints; j++) {
			if (!maybeRead(&curObst.xs[j], sizeof(curObst.xs[j]),
					file))
				goto f_curObst;
			if (!maybeRead(&curObst.ys[j], sizeof(curObst.ys[j]),
					file))
				goto f_curObst;
		}
		if (obsts) (*obsts)[i] = curObst;
	}

	if (argNumObsts) *argNumObsts = numObsts;

	return true;

f_curObst:
	if (obsts) {
		for (size_t i = 0; i < numObsts; i++) {
			if ((*obsts)[i].xs) free((*obsts)[i].xs);
			if ((*obsts)[i].ys) free((*obsts)[i].ys);
		}
		free(*obsts);
	}
f_obsts:
f_numObsts:
	return false;
}

bool LevelIO_Read(
		const char *path,
		LevelIO_Hole *hole,
		LevelIO_Proj *proj,
		Tile (**tiles)[LEVEL_HEIGHT_TILES],
		Tile_WithPos **overlayTiles,
		size_t *numOverlayTiles,
		LevelIO_Obst **obstacles,
		size_t *numObsts,
		int *width,
		int *par,
		char **name
	) {
	FILE *data = fopen(path, "rb");
	if (!data) goto f_data;

	// We use these values, so make sure there is space allocated for them
	int readWidth, projNum;
	size_t nameSize, overlayTilesSize;

	if (!maybeRead(&readWidth, sizeof(readWidth), data)) goto f_maybeRead1;
	if (!maybeRead(par, sizeof(*par), data)) goto f_maybeRead1;

	size_t tilesSize = sizeof(**tiles) * readWidth / TILE_SIZE;
	if (tiles) {
		*tiles = malloc(tilesSize);
		if (!(*tiles)) goto f_tiles;
	}

	if (!maybeRead(hole  ? &hole->x      : NULL, sizeof(hole->x),      data))
		goto f_maybeRead2;
	if (!maybeRead(hole  ? &hole->y      : NULL, sizeof(hole->y),      data))
		goto f_maybeRead2;
	if (!maybeRead(hole  ? &hole->width  : NULL, sizeof(hole->width),  data))
		goto f_maybeRead2;
	if (!maybeRead(hole  ? &hole->height : NULL, sizeof(hole->height), data))
		goto f_maybeRead2;
	if (!maybeRead(proj  ? &proj->startX : NULL, sizeof(proj->startX), data))
		goto f_maybeRead2;
	if (!maybeRead(proj  ? &proj->startY : NULL, sizeof(proj->startY), data))
		goto f_maybeRead2;
	if (!maybeRead(&projNum,                     sizeof(projNum),      data))
		goto f_maybeRead2;
	if (!maybeRead(tiles ? **tiles       : NULL, tilesSize,            data))
		goto f_maybeRead2;
	if (!maybeRead(&nameSize,                    sizeof(nameSize),     data))
		goto f_maybeRead2;

	if (name) {
		*name = malloc(nameSize);
		if (!(*name)) goto f_name;
	}

	if (!maybeRead(name ? *name : NULL, nameSize, data)) goto f_maybeRead3;
	if (!maybeRead(&overlayTilesSize, sizeof(overlayTilesSize), data))
		goto f_maybeRead3;

	if (overlayTiles) {
		*overlayTiles = malloc(overlayTilesSize);
		if (!(*overlayTiles)) goto f_overlayTiles;
	}

	if (!maybeRead(overlayTiles ? *overlayTiles : NULL, overlayTilesSize, data))
		goto f_maybeRead4;
	if (!readObstacles(obstacles, numObsts, data)) goto f_maybeRead4;

	if (proj) {
		proj->type = numToProj(projNum);
		if (!proj->type) goto f_projtype;
	}
	if (width) *width = readWidth;
	if (numOverlayTiles) {
		*numOverlayTiles = overlayTilesSize / sizeof(**overlayTiles);
	}

	fclose(data);
	return true;

f_projtype:
f_maybeRead4:
	if (overlayTiles) free(*overlayTiles);
f_overlayTiles:
f_maybeRead3:
	if (name) free(*name);
f_name:
f_maybeRead2:
	if (tiles) free(*tiles);
f_tiles:
f_maybeRead1:
	fclose(data);
f_data:
	return false;
}

static bool writeObstacles(LevelIO_Obst *obsts, size_t numObsts, FILE *file) {
	if (!fwrite(&numObsts, sizeof(numObsts), 1, file)) return false;
	for (size_t i = 0; i < numObsts; i++) {
		if (!fwrite(&obsts[i].sprite1,   sizeof(obsts[i].sprite1),  1, file))
			return false;
		if (!fwrite(&obsts[i].sprite2,   sizeof(obsts[i].sprite2),  1, file))
			return false;
		if (!fwrite(&obsts[i].speed,     sizeof(obsts[i].speed),    1, file))
			return false;
		if (!fwrite(&obsts[i].numPoints, sizeof(obsts[i].numPoints), 1,file))
			return false;

		for (int j = 0; j < obsts[i].numPoints; j++) {
			if (!fwrite(&obsts[i].xs[j], sizeof(obsts[i].xs[j]), 1,file))
				return false;
			if (!fwrite(&obsts[i].ys[j], sizeof(obsts[i].ys[j]), 1,file))
				return false;
		}
	}
	return true;
}

bool LevelIO_Write(
		const char *path,
		LevelIO_Hole hole,
		LevelIO_Proj proj,
		const Tile (*tiles)[LEVEL_HEIGHT_TILES],
		const Tile_WithPos *overlayTiles,
		size_t numOverlayTiles,
		LevelIO_Obst *obstacles,
		size_t numObsts,
		int width,
		int par,
		const char *name
	) {
	FILE *data = fopen(path, "wb");
	if (!data) goto f_data;

	size_t tilesSize = sizeof(*tiles) * width / TILE_SIZE;
	int projNum = projToNum(proj.type);

	size_t nameSize = (strlen(name) + 1) * sizeof(char);
	size_t overlayTilesSize = numOverlayTiles * sizeof(*overlayTiles);

	if (!fwrite(&width,            sizeof(width),       1, data)) goto f_fwrite;
	if (!fwrite(&par,              sizeof(par),         1, data)) goto f_fwrite;
	if (!fwrite(&hole.x,           sizeof(hole.x),      1, data)) goto f_fwrite;
	if (!fwrite(&hole.y,           sizeof(hole.y),      1, data)) goto f_fwrite;
	if (!fwrite(&hole.width,       sizeof(hole.width),  1, data)) goto f_fwrite;
	if (!fwrite(&hole.height,      sizeof(hole.height), 1, data)) goto f_fwrite;
	if (!fwrite(&proj.startX,      sizeof(proj.startX), 1, data)) goto f_fwrite;
	if (!fwrite(&proj.startY,      sizeof(proj.startY), 1, data)) goto f_fwrite;
	if (!fwrite(&projNum,          sizeof(projNum),     1, data)) goto f_fwrite;
	if (!fwrite(tiles,             tilesSize,           1, data)) goto f_fwrite;
	if (!fwrite(&nameSize,         sizeof(nameSize),    1, data)) goto f_fwrite;
	if (!fwrite(name,              nameSize,            1, data)) goto f_fwrite;
	if (!fwrite(&overlayTilesSize, sizeof(overlayTilesSize), 1, data))
		goto f_fwrite;
	if (!fwrite(overlayTiles,      overlayTilesSize,    1, data)) goto f_fwrite;
	if (!writeObstacles(obstacles, numObsts,               data)) goto f_fwrite;

	fclose(data);
	return true;

f_fwrite:
	fclose(data);
f_data:
	return false;
}

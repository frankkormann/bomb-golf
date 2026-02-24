/*
 * Reading/writing of level files.
 */

#ifndef LEVELIO_H
#define LEVELIO_H

#include <stdbool.h>
#include "rendering/background.h"
#include "projectile.h"

#define LEVEL_HEIGHT 240
#define LEVEL_MAX_WIDTH 1020

#define LEVEL_MAX_WIDTH_TILES (LEVEL_MAX_WIDTH / BG_TILE_SIZE)
#define LEVEL_HEIGHT_TILES (LEVEL_HEIGHT / BG_TILE_SIZE)

typedef struct {
	int x;
	int y;
	int width;
	int height;
} LevelIO_Hole;

typedef struct {
	int startX;
	int startY;
	Projectile type;
} LevelIO_Proj;

/*
 * Reads the level file at path and fills in *proj, *width, and *tiles with the
 * data therein. *tiles is allocated with length *width.
 *
 * Returns false if an error occurred. *tiles will not be allocated if false is
 * returned.
 */
bool LevelIO_Read(const char *path, LevelIO_Hole *hole, LevelIO_Proj *proj,
		BG_Tile (**tiles)[LEVEL_HEIGHT_TILES], int *width);

/*
 * Writes a level to the file at path, overwriting it.
 *
 * Returns false if an error occurred.
 */
bool LevelIO_Write(const char *path, LevelIO_Hole hole, LevelIO_Proj proj,
		const BG_Tile (*tiles)[LEVEL_HEIGHT_TILES], int width);
#endif

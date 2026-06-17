#include <stdbool.h>
#include <limits.h>
#include <malloc.h>
#include "terrain.h"
#include "tile.h"
#include "scenes/components/background.h"
#include "rendering/colors.h"
#include "rendering/spritesheet.h"
#include "rendering/animation.h"
#include "rendering/animations/explosion.h"
#include "util/queue.h"

// (x, y) goes to [x + y*width]
Terrain_Type *typeMap;
int width, height;
Background bg;
Queue tilesToExplode;

bool Terrain_Init(int argWidth, int argHeight) {
	typeMap = calloc(argWidth * argHeight, sizeof(*typeMap));
	if (!typeMap) return false;

	bg = BG_Create(argWidth, argHeight, COLOR_BLUE);
	if (!bg) {
		free(typeMap);
		return false;
	}

	tilesToExplode = Queue_Create();
	if (!tilesToExplode) {
		free(typeMap);
		BG_Free(bg);
		return false;
	}

	width = argWidth;
	height = argHeight;

	return true;
}

void Terrain_Exit() {
	free(typeMap);
	BG_Free(bg);
	Queue_Free(tilesToExplode);
}

Terrain_Type getTerrainForTile(Tile tile) {
	switch (Tile_GetSprite(tile)) {
		default:
			return TERRAIN_GROUND;
		case SPRITE_TILE_OVERLAY_BOUNCY:
		case SPRITE_TILE_OVERLAY_BOUNCY_TRIANGLE:
			return TERRAIN_BOUNCY;
		case SPRITE_TILE_EXPLOSIVE:
			return TERRAIN_EXPLOSIVE;
	}
}

static void setTerrainForHalf(int x, int y, u8 orientation, Terrain_Type type) {
	switch (orientation) {
		case 0:
		case TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE / 2; i++) {
					typeMap[x+i + (y+j)*width] = type;
				}
			}
			break;
		case TILE_ROTATE_90:
		case TILE_ROTATE_90 | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE/2; j++) {
				for (int i = 0; i < TILE_SIZE; i++) {
					typeMap[x+i + (y+j)*width] = type;
				}
			}
			break;
		case TILE_FLIP_HORIZ:
		case TILE_FLIP_HORIZ | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = TILE_SIZE/2; i < TILE_SIZE; i++) {
					typeMap[x+i + (y+j)*width] = type;
				}
			}
			break;
		case TILE_ROTATE_90 | TILE_FLIP_HORIZ:
		case TILE_ROTATE_90 | TILE_FLIP_HORIZ | TILE_FLIP_VERT:
			for (int j = TILE_SIZE/2; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE; i++) {
					typeMap[x+i + (y+j)*width] = type;
				}
			}
			break;
	}
}

static void setTerrainForTriangle(int x, int y, u8 orientation, Terrain_Type type) {
	switch (orientation) {
		case 0:
		case TILE_ROTATE_90 | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = j; i < TILE_SIZE; i++) {
					typeMap[x+i +(TILE_SIZE+y-j-1)*width] = type;
				}
			}
			break;
		case TILE_FLIP_VERT:
		case TILE_ROTATE_90 | TILE_FLIP_HORIZ | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = j; i < TILE_SIZE; i++) {
					typeMap[x+i + (y+j)*width] = type;
				}
			}
			break;
		case TILE_FLIP_HORIZ:
		case TILE_ROTATE_90:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE - j; i++) {
					typeMap[x+i +(TILE_SIZE+y-j-1)*width] = type;
				}
			}
			break;
		case TILE_ROTATE_90 | TILE_FLIP_HORIZ:
		case TILE_FLIP_HORIZ | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE - j; i++) {
					typeMap[x+i + (y+j)*width] = type;
				}
			}
			break;
	}
}

void Terrain_FillTile(int x, int y, Tile tile, bool clearPrevious) {
	if (clearPrevious) {
		for (int j = 0; j < TILE_SIZE; j++) {
			for (int i = 0; i < TILE_SIZE; i++) {
				typeMap[x+i + (y+j)*width] = TERRAIN_NOTHING;
			}
		}
	}
	Terrain_Type type = getTerrainForTile(tile);
	switch (Tile_GetHitbox(tile)) {
		case TILE_HITBOX_NONE:
			break;
		case TILE_HITBOX_FULL:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE; i++) {
					typeMap[x+i + (y+j)*width] = type;
				}
			}
			break;
		case TILE_HITBOX_HALF:
			setTerrainForHalf(x, y, Tile_GetOrientFlags(tile), type);
			break;
		case TILE_HITBOX_TRIANGLE:
			setTerrainForTriangle(x, y, Tile_GetOrientFlags(tile), type);
			break;
	}
	BG_DrawTile(bg, tile, x, y, clearPrevious);
}

void Terrain_ClearPixel(int x, int y) {
	if (typeMap[x + y*width] == TERRAIN_EXPLOSIVE) {
		Queue_Push(tilesToExplode, (void*)(x + y*width));
	} else {
		typeMap[x + y*width] = TERRAIN_NOTHING;
		BG_ClearPixel(bg, x, y);
	}
}

void Terrain_ClearCircle(int x, int y, int radius) {
	//https://stackoverflow.com/a/24453110
	int r2 = radius * radius;
	int area = r2 << 2;
	int rr = radius << 1;

	for (int i = 0; i < area; i++) {
		int tx = (i % rr) - radius;
		int ty = (i / rr) - radius;
		int nx = x + tx;
		int ny = y + ty;
		if (tx * tx + ty * ty <= r2 && nx >= 0 && nx < width && ny >= 0
				&& ny < height) {
			Terrain_ClearPixel(nx, ny);
		}
	}
}

Terrain_Type Terrain_TypeAt(int x, int y) {
	if (x < 0 || x >= width || y < 0 || y >= height) return TERRAIN_GROUND;
	return typeMap[x + y*width];
}

static void explodeTile(int x, int y) {
	if (typeMap[x + y*width] != TERRAIN_EXPLOSIVE) return;

	int gridX = (x / TILE_SIZE) * TILE_SIZE;
	int gridY = (y / TILE_SIZE) * TILE_SIZE;
	// Fill the tile with TERRAIN_GROUND so consecutive calls for the same tile
	// do nothing
	for (int i = gridX; i < gridX + TILE_SIZE; i++) {
		for (int j = gridY; j < gridY + TILE_SIZE; j++) {
			typeMap[i + j*width] = TERRAIN_GROUND;
		}
	}

	int midX = gridX + TILE_SIZE/2;
	int midY = gridY + TILE_SIZE/2;
	int radius = (TILE_SIZE/2) * 1.41 + 1;
	Animation_Start(animationExplosion,
			Explosion_MakeParams(midX, midY, radius), NULL);
	Terrain_ClearCircle(midX, midY, radius);
}

void Terrain_Update() {
	while (!Queue_IsEmpty(tilesToExplode)) {
		int pos = (int)Queue_Pop(tilesToExplode);
		explodeTile(pos % width, pos / width);
	}
}

void Terrain_UpdateGraphics() {
	BG_UpdateGraphics(bg);
}

void Terrain_Draw(int x, int y, float depth, int maxWidth, int maxHeight,
		int *drawnX, int *drawnY, int *drawnWidth, int *drawnHeight) {
	BG_DrawFit(bg, x, y, depth, maxWidth, maxHeight, drawnX, drawnY,
			drawnWidth, drawnHeight);
}

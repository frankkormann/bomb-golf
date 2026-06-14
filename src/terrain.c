#include <stdbool.h>
#include <limits.h>
#include <malloc.h>
#include "terrain.h"
#include "tile.h"
#include "scenes/components/background.h"
#include "rendering/colors.h"
#include "rendering/spritesheet.h"
#include "util/queue.h"

#define EXPLOSIVES_QUEUE_SENTINEL -1

// (x, y) goes to [x + y*width]
Terrain_Type *typeMap;
int width, height;
Background bg;
Queue explosivesQueue;

bool Terrain_Init(int argWidth, int argHeight) {
	typeMap = calloc(argWidth * argHeight, sizeof(*typeMap));
	if (!typeMap) return false;

	bg = BG_Create(argWidth, argHeight, COLOR_BLUE);
	if (!bg) {
		free(typeMap);
		return false;
	}

	explosivesQueue = Queue_Create();
	if (!explosivesQueue) {
		free(typeMap);
		BG_Free(bg);
		return false;
	}
	Queue_Push(explosivesQueue, (void*)EXPLOSIVES_QUEUE_SENTINEL);

	width = argWidth;
	height = argHeight;

	return true;
}

void Terrain_Exit() {
	free(typeMap);
	BG_Free(bg);
	Queue_Free(explosivesQueue);
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
					typeMap[x+i + (y+j)] = type;
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
	typeMap[x + y*width] = TERRAIN_NOTHING;
	BG_ClearPixel(bg, x, y);

	if (x > 0 && typeMap[(x-1) + y*width] == TERRAIN_EXPLOSIVE) {
		typeMap[(x-1) + y*width] = TERRAIN_GROUND;
		Queue_Push(explosivesQueue, (void*)((x-1) + y*width));
	}
	if (x < width-1 && typeMap[(x+1) + y*width] == TERRAIN_EXPLOSIVE) {
		typeMap[(x+1) + y*width] = TERRAIN_GROUND;
		Queue_Push(explosivesQueue, (void*)((x+1) + y*width));
	}
	if (y > 0 && typeMap[x + (y-1)*width] == TERRAIN_EXPLOSIVE) {
		typeMap[x + (y-1)*width] = TERRAIN_GROUND;
		Queue_Push(explosivesQueue, (void*)(x + (y-1)*width));
	}
	if (y < height-1 && typeMap[x + (y+1)*width] == TERRAIN_EXPLOSIVE) {
		typeMap[x + (y+1)*width] = TERRAIN_GROUND;
		Queue_Push(explosivesQueue, (void*)(x + (y+1)*width));
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

void Terrain_Update() {
	// Queue is prefilled with an EXPLOSIVES_QUEUE_SENTINEL and we re-add it
	// every time, so no need to check if it's empty
	int pos;
	while (pos = (int)Queue_Pop(explosivesQueue),
			pos != EXPLOSIVES_QUEUE_SENTINEL) {
		Terrain_ClearPixel(pos % width, pos / width);
	}
	Queue_Push(explosivesQueue, (void*)EXPLOSIVES_QUEUE_SENTINEL);
}

void Terrain_UpdateGraphics() {
	BG_UpdateGraphics(bg);
}

void Terrain_Draw(int x, int y, float depth, int maxWidth, int maxHeight,
		int *drawnX, int *drawnY, int *drawnWidth, int *drawnHeight) {
	BG_DrawFit(bg, x, y, depth, maxWidth, maxHeight, drawnX, drawnY,
			drawnWidth, drawnHeight);
}

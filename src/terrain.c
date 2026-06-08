#include <stdbool.h>
#include <malloc.h>
#include "terrain.h"
#include "tile.h"
#include "scenes/components/background.h"
#include "rendering/colors.h"
#include "rendering/spritesheet.h"

// (x, y) goes to [x + y*width]
Terrain_Type *typeMap;
int width, height;
Background bg;

bool Terrain_Init(int argWidth, int argHeight) {
	typeMap = calloc(argWidth * argHeight, sizeof(*typeMap));
	if (!typeMap) return false;

	bg = BG_Create(argWidth, argHeight, COLOR_BLUE);
	if (!bg) {
		free(typeMap);
		return false;
	}

	width = argWidth;
	height = argHeight;

	return true;
}

void Terrain_Exit() {
	free(typeMap);
	BG_Free(bg);
}

Terrain_Type getTerrainForTile(Tile tile) {
	SpriteSheet_TileSprite sprite = Tile_GetSprite(tile);
	if (sprite == SPRITE_TILE_OVERLAY_BOUNCY) {
		return TERRAIN_BOUNCY;
	} else {
		return TERRAIN_GROUND;
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
			typeMap[nx + ny*width] = TERRAIN_NOTHING;
			BG_ClearPixel(bg, nx, ny);
		}
	}
}

Terrain_Type Terrain_TypeAt(int x, int y) {
	if (x < 0 || x >= width || y < 0 || y >= height) return TERRAIN_GROUND;
	return typeMap[x + y*width];
}

void Terrain_UpdateGraphics() {
	BG_UpdateGraphics(bg);
}

void Terrain_Draw(int x, int y, float depth, int maxWidth, int maxHeight,
		int *drawnX, int *drawnY, int *drawnWidth, int *drawnHeight) {
	BG_DrawFit(bg, x, y, depth, maxWidth, maxHeight, drawnX, drawnY,
			drawnWidth, drawnHeight);
}

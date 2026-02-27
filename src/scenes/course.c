#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "../scene.h"
#include "scene_internal.h"
#include "course.h"
#include "title.h"
#include "../projectile.h"
#include "../projectiles/ball.h"
#include "../rendering/rendertarget.h"
#include "../rendering/background.h"
#include "../util/touchinput.h"
#include "../levelio.h"

#define NUM_LEVELS 5
#define LAUNCH_SPEED_MAX 6
#define SKY_COLOR (C2D_Color32(4, 132, 209, 255))

static unsigned int level;
static bool isSdmc;
static bool (*terrain)[LEVEL_HEIGHT];

static Background bg;
static int holeX, holeY, holeWidth, holeHeight;
static int fieldWidth;

static unsigned int strokeCounter;

static C2D_Text infoText;
static C2D_TextBuf textBuf;

static bool withinBounds(int x, int y) {
	return 0 <= x && x <= fieldWidth-1 && 0 <= y && y <= LEVEL_HEIGHT-1;
}

int Course_GetFieldWidth() {
	return fieldWidth;
}

int Course_GetFieldHeight() {
	return LEVEL_HEIGHT;
}

bool Course_CheckTerrain(int x, int y) {
	return !withinBounds(x, y) || terrain[x][y];
}

void Course_ClearCircle(int x, int y, int radius) {
	//https://stackoverflow.com/a/24453110
	int r2 = radius * radius;
	int area = r2 << 2;
	int rr = radius << 1;

	for (int i = 0; i < area; i++) {
		int tx = (i % rr) - radius;
		int ty = (i / rr) - radius;

		int nx = x + tx;
		int ny = y + ty;
		if (tx * tx + ty * ty <= r2 && withinBounds(nx, ny)) {
			terrain[nx][ny] = false;
			if (!BG_ClearPixel(bg, nx, ny)) {
				strokeCounter += 10;
			}
		}
	}
}

static void setTerrainForTile(BG_Tile tile, int x, int y) {
	switch (tile) {
		case TILE_CLEAR:
			break;
		case TILE_GRASS_TOP:
		case TILE_DIRT_TOP_N:
		case TILE_DIRT_TOP_S:
		case TILE_DIRT_TOP_W:
		case TILE_DIRT_TOP_E:
		case TILE_DIRT_INTERNAL:
		case TILE_DIRT_TRI_FILL_NW:
		case TILE_DIRT_TRI_FILL_NE:
		case TILE_DIRT_TRI_FILL_SW:
		case TILE_DIRT_TRI_FILL_SE:
		case TILE_GRASS_TRI_FILL_W:
		case TILE_GRASS_TRI_FILL_E:
			for (int j = 0; j < BG_TILE_SIZE; j++) {
				for (int i = 0; i < BG_TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_GRASS_TRI_NW:
		case TILE_DIRT_TRI_NW:
			for (int j = 0; j < BG_TILE_SIZE; j++) {
				for (int i = 0; i < BG_TILE_SIZE - j; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_GRASS_TRI_NE:
		case TILE_DIRT_TRI_NE:
			for (int j = 0; j < BG_TILE_SIZE; j++) {
				for (int i = j; i < BG_TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_GRASS_TRI_SW:
		case TILE_DIRT_TRI_SW:
			for (int j = 0; j < BG_TILE_SIZE; j++) {
				for (int i = 0; i < BG_TILE_SIZE - j; i++) {
					terrain[x+i][BG_TILE_SIZE+y-j-1] = true;
				}
			}
			break;
		case TILE_GRASS_TRI_SE:
		case TILE_DIRT_TRI_SE:
			for (int j = 0; j < BG_TILE_SIZE; j++) {
				for (int i = j; i < BG_TILE_SIZE; i++) {
					terrain[x+i][BG_TILE_SIZE+y-j-1] = true;
				}
			}
			break;
		case TILE_GRASS_HALF_N:
			for (int j = 0; j < BG_TILE_SIZE/2; j++) {
				for (int i = 0; i < BG_TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_GRASS_HALF_S:
			for (int j = BG_TILE_SIZE/2; j < BG_TILE_SIZE; j++) {
				for (int i = 0; i < BG_TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_GRASS_HALF_W:
			for (int j = 0; j < BG_TILE_SIZE; j++) {
				for (int i = 0; i < BG_TILE_SIZE / 2; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_GRASS_HALF_E:
			for (int j = 0; j < BG_TILE_SIZE; j++) {
				for (int i = BG_TILE_SIZE/2; i < BG_TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case NUM_TILES:
			// Satisfy compiler warning
			break;
	}
}

static bool loadLevel(char path[]) {
	LevelIO_Hole hole;
	LevelIO_Proj proj;
	BG_Tile (*tiles)[LEVEL_HEIGHT_TILES];
	if (!LevelIO_Read(path, &hole, &proj, &tiles, &fieldWidth)) return false;

	terrain = malloc(sizeof(*terrain) * fieldWidth);
	if (!terrain) goto failed;

	for (int x = 0; x < fieldWidth; x++) {
		for (int y = 0; y < LEVEL_HEIGHT; y++) {
			terrain[x][y] = false;
		}
	}

	bg = BG_Create(fieldWidth, LEVEL_HEIGHT, SKY_COLOR);
	if (!bg) goto failed;

	for (int x = 0; x < fieldWidth / BG_TILE_SIZE; x++) {
		for (int y = 0; y < LEVEL_HEIGHT / BG_TILE_SIZE; y++)  {
			if (tiles[x][y] == TILE_CLEAR) continue;
			setTerrainForTile(tiles[x][y], x*BG_TILE_SIZE,
					y*BG_TILE_SIZE);
			BG_DrawTile(bg, tiles[x][y], x*BG_TILE_SIZE,
					y*BG_TILE_SIZE, false);
		}
	}
	free(tiles);

	holeX = hole.x;
	holeY = hole.y;
	holeWidth = hole.width;
	holeHeight = hole.height;

	Projectile_SetType(proj.type);
	Projectile_SetPos(proj.startX, proj.startY);

	return true;

failed:
	if (tiles) free(tiles);
	if (terrain) free(terrain);
	if (bg) BG_Free(bg);
	return false;
}

Scene_Params Course_MakeParams(unsigned int level, bool isSdmc) {
	return (Scene_Params) { .course = {
		.level = level,
		.isSdmc = isSdmc
	} };
}

static bool sceneInit(Scene_Params params) {
	char path[20];
	sprintf(path, "%s:/level_%i.bin", params.course.isSdmc ? "sdmc" : "romfs",
			params.course.level);
	isSdmc = params.course.isSdmc;
	if (!loadLevel(path)) return false;

	textBuf = C2D_TextBufNew(256);
	if (!textBuf) {
		BG_Free(bg);
		return false;
	}

	strokeCounter = 0;
	level = params.course.level;

	return true;
}

static void calculateLaunchVelocity(TouchInput_Swipe stroke, float *velX,
		float *velY) {
	*velX = (float)(stroke.end.px - stroke.start.px) / stroke.length;
	*velY = (float)(stroke.end.py - stroke.start.py) / stroke.length;
	float magnitude² = *velX * *velX + *velY * *velY;
	if (magnitude² > LAUNCH_SPEED_MAX*LAUNCH_SPEED_MAX) {
		// Set vector length to LAUNCH_SPEED_MAX
		*velX *= LAUNCH_SPEED_MAX / sqrt(magnitude²);
		*velY *= LAUNCH_SPEED_MAX / sqrt(magnitude²);
	}
}

static void checkLaunchInput() {
	if (TouchInput_JustFinished()) {
		TouchInput_Swipe stroke = TouchInput_GetSwipe();
		float velX, velY;
		calculateLaunchVelocity(stroke, &velX, &velY);
		Projectile_Launch(velX, velY);

		strokeCounter++;
	}
}

static void nextLevel() {
	level++;
	if (!isSdmc && level > NUM_LEVELS) {
		Scene_SetNext(sceneTitle, Title_MakeParams());
		return;
	}
	Scene_SetNext(sceneCourse, Course_MakeParams(level, isSdmc));
}

static void sceneUpdate() {
	if (BG_IsUpdating(bg)) return;

	u32 kDown = hidKeysDown();
//	u32 kHeld = hidKeysHeld();

	if (kDown & KEY_B) {
		Scene_SetNext(sceneTitle, Title_MakeParams());
		return;
	}

	if (!Projectile_IsMoving()) {
		checkLaunchInput();
	}
	
	// Check in hole before updating so there is 1 visible frame where
	// the projectile is in the hole
	float x, y;
	Projectile_GetPos(&x, &y);
	if (holeX <= x && x <= holeX + holeWidth && holeY <= y
			&& y <= holeY + holeHeight) {
		nextLevel();
	}

	Projectile_Update();
}

static void layoutInfoText() {
	C2D_TextBufClear(textBuf);
	char cbuf[256];
	sprintf(cbuf, "Strokes: %i", strokeCounter);
	C2D_TextParse(&infoText, textBuf, cbuf);
	C2D_TextOptimize(&infoText);
}

static void sceneDraw() {
	BG_UpdateGraphics(bg);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, C2D_Color32(255, 255, 255, 255));
	C2D_SceneBegin(bottom);

	Projectile_CenterViewC2D(GFX_BOTTOM);

	if (!Projectile_IsMoving() && TouchInput_InProgress()) {
		TouchInput_Swipe stroke = TouchInput_GetSwipe();
		float velX, velY;
		calculateLaunchVelocity(stroke, &velX, &velY);

		float strength = (velX*velX + velY*velY)
				/ (LAUNCH_SPEED_MAX*LAUNCH_SPEED_MAX);
		int r = 512 * strength;
		int g = 512 - r;
		r = r > 255 ? 255 : r;
		g = g > 255 ? 255 : g;
		u32 color = C2D_Color32(r, g, 0, 255);

		float projX, projY;
		Projectile_GetPos(&projX, &projY);
		float strokeX = stroke.end.px - stroke.start.px;
		float strokeY = stroke.end.py - stroke.start.py;

		C2D_DrawLine(projX, projY, color, projX + strokeX, projY + strokeY,
				color, 2, 1);
	}

	BG_Draw(bg, 0, 0, -1, 1, 1);
	Projectile_Draw(0);

	C2D_ViewReset();


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, C2D_Color32(255, 255, 255, 255));
	C2D_SceneBegin(top);

	float scale = 400.0 / fieldWidth;
	float bgY = 120 - (LEVEL_HEIGHT * scale) / 2;
	BG_Draw(bg, 0, bgY, 0, scale, scale);

	if (!Projectile_IsMoving()) {
		float x, y;
		Projectile_GetPos(&x, &y);
		C2D_DrawRectSolid(x * scale, bgY + y * scale, 1, 2, 2,
				C2D_Color32(255, 255, 255, 255));
	}

	layoutInfoText();
	C2D_DrawText(&infoText, 0, 10, 20, 0, 0.5, 0.5);
}

static void sceneExit() {
	if (bg) BG_Free(bg);
	if (textBuf) C2D_TextBufDelete(textBuf);
	if (terrain) free(terrain);
}

Scene sceneCourse = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

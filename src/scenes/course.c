#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "../scene.h"
#include "scene_internal.h"
#include "course.h"
#include "title.h"
#include "components/text.h"
#include "../projectile.h"
#include "../projectiles/ball.h"
#include "../rendering/colors.h"
#include "../rendering/rendertarget.h"
#include "../rendering/background.h"
#include "../rendering/animation.h"
#include "../rendering/animations/firework.h"
#include "../util/touchinput.h"
#include "../util/macros.h"
#include "../levelio.h"

#define LAUNCH_SPEED_MAX 6
#define TOUCHSCREEN_TO_LAUNCH_VEL_FACTOR 0.05

static unsigned int level;
static bool isSdmc;
static bool (*terrain)[LEVEL_HEIGHT];

static Background bg;
static int holeX, holeY, holeWidth, holeHeight;
static int fieldWidth;

static unsigned int strokeCounter;
static int par;
static bool hasFinished;

static Text infoText;

static bool withinBounds(int x, int y) {
	return 0 <= x && x <= fieldWidth-1 && 0 <= y && y <= LEVEL_HEIGHT-1;
}

int Course_GetFieldWidth() {
	return fieldWidth;
}

int Course_GetFieldHeight() {
	return LEVEL_HEIGHT;
}

int Course_GetScreenOffset() {
	float projX, projY;
	Projectile_GetPos(&projX, &projY);
	return clamp(projX - 160, 0, fieldWidth - 320);
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

static void setTerrainForHalf(u8 orientation, int x, int y) {
	switch(orientation) {
		case 0:
		case TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE / 2; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_ROTATE_90:
		case TILE_ROTATE_90 | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE/2; j++) {
				for (int i = 0; i < TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_FLIP_HORIZ:
		case TILE_FLIP_HORIZ | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = TILE_SIZE/2; i < TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_ROTATE_90 | TILE_FLIP_HORIZ:
		case TILE_ROTATE_90 | TILE_FLIP_HORIZ | TILE_FLIP_VERT:
			for (int j = TILE_SIZE/2; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
	}
}

static void setTerrainForTriangle(u8 orientation, int x, int y) {
	switch (orientation) {
		case 0:
		case TILE_ROTATE_90 | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = j; i < TILE_SIZE; i++) {
					terrain[x+i][TILE_SIZE+y-j-1] = true;
				}
			}
			break;
		case TILE_FLIP_VERT:
		case TILE_ROTATE_90 | TILE_FLIP_HORIZ | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = j; i < TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_FLIP_HORIZ:
		case TILE_ROTATE_90:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE - j; i++) {
					terrain[x+i][TILE_SIZE+y-j-1] = true;
				}
			}
			break;
		case TILE_ROTATE_90 | TILE_FLIP_HORIZ:
		case TILE_FLIP_HORIZ | TILE_FLIP_VERT:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE - j; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
	}
}

static void setTerrainForTile(Tile tile, int x, int y) {
	switch (Tile_GetHitbox(tile)) {
		case TILE_HITBOX_NONE:
			break;
		case TILE_HITBOX_FULL:
			for (int j = 0; j < TILE_SIZE; j++) {
				for (int i = 0; i < TILE_SIZE; i++) {
					terrain[x+i][y+j] = true;
				}
			}
			break;
		case TILE_HITBOX_HALF:
			setTerrainForHalf(Tile_GetOrientFlags(tile), x, y);
			break;
		case TILE_HITBOX_TRIANGLE:
			setTerrainForTriangle(Tile_GetOrientFlags(tile), x, y);
			break;
	}
}

static bool loadLevel(char path[]) {
	LevelIO_Hole hole;
	LevelIO_Proj proj;
	Tile (*tiles)[LEVEL_HEIGHT_TILES];
	if (!LevelIO_Read(path, &hole, &proj, &tiles, &fieldWidth, &par)) {
		return false;
	}

	terrain = malloc(sizeof(*terrain) * fieldWidth);
	if (!terrain) goto failed;

	for (int x = 0; x < fieldWidth; x++) {
		for (int y = 0; y < LEVEL_HEIGHT; y++) {
			terrain[x][y] = false;
		}
	}

	bg = BG_Create(fieldWidth, LEVEL_HEIGHT, COLOR_BLUE);
	if (!bg) goto failed;

	for (int x = 0; x < fieldWidth / TILE_SIZE; x++) {
		for (int y = 0; y < LEVEL_HEIGHT / TILE_SIZE; y++)  {
			setTerrainForTile(tiles[x][y], x*TILE_SIZE,
					y*TILE_SIZE);
			if (Tile_GetSprite(tiles[x][y]) != SPRITE_TILE_SKY) {
				BG_DrawTile(bg, tiles[x][y], x*TILE_SIZE,
						y*TILE_SIZE, false);
			}
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
	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(params.course.level, params.course.isSdmc, path);
	isSdmc = params.course.isSdmc;
	if (!loadLevel(path)) return false;

	infoText = Text_Create(100, NULL);
	if (!infoText) {
		BG_Free(bg);
		return false;
	}

	strokeCounter = 0;
	hasFinished = false;
	level = params.course.level;

	return true;
}

static void calculateLaunchVelocity(float *velX, float *velY) {
	TouchInput_Swipe stroke = TouchInput_GetSwipe();
	float projX, projY;
	Projectile_GetPos(&projX, &projY);
	*velX = (float)(stroke.end.px - projX + Course_GetScreenOffset())
			* TOUCHSCREEN_TO_LAUNCH_VEL_FACTOR;
	*velY = (float)(stroke.end.py - projY) * TOUCHSCREEN_TO_LAUNCH_VEL_FACTOR;
	float magnitude² = *velX * *velX + *velY * *velY;
	if (magnitude² > LAUNCH_SPEED_MAX*LAUNCH_SPEED_MAX) {
		// Set vector length to LAUNCH_SPEED_MAX
		*velX *= LAUNCH_SPEED_MAX / sqrt(magnitude²);
		*velY *= LAUNCH_SPEED_MAX / sqrt(magnitude²);
	}
}

static bool canLaunch() {
	return !Projectile_IsMoving() && !hasFinished;
}

static void checkLaunchInput() {
	if (TouchInput_JustFinished()) {
		float velX, velY;
		calculateLaunchVelocity(&velX, &velY);
		Projectile_Launch(velX, velY);

		strokeCounter++;
	}
}

static void nextLevel() {
	level++;
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

	if (canLaunch()) {
		checkLaunchInput();
	}
	
	float x, y;
	Projectile_GetPos(&x, &y);
	if (!hasFinished && holeX <= x && x <= holeX + holeWidth && holeY <= y
			&& y <= holeY + holeHeight) {
		hasFinished = true;
		if (!Animation_Start(animationFirework, Firework_MakeParams(x, y),
				nextLevel)) {
			nextLevel();
		}
	}

	Projectile_Update();

	Text_SetContent(infoText, "Strokes: %i\nPar:       %i", strokeCounter, par);
}

static void plotTrajectoryPoint(float initX, float initY, float velX, float velY,
		int framesInFuture, float size, float depth, u32 color) {
	float pointX = initX + (velX * framesInFuture);
	float pointY = initY + (velY * framesInFuture)
			+ (0.5 * PROJECTILE_GRAVITY * framesInFuture*framesInFuture);
	C2D_DrawRectSolid(pointX - size/2, pointY - size/2, depth, size, size,
			color);
}

static void sceneDraw() {
	BG_UpdateGraphics(bg);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_WHITE);
	C2D_SceneBegin(bottom);

	C2D_ViewTranslate(-Course_GetScreenOffset(), 0);

	if (canLaunch() && TouchInput_InProgress()) {
		float projX, projY, velX, velY;
		calculateLaunchVelocity(&velX, &velY);
		Projectile_GetPos(&projX, &projY);

		float strength = (velX*velX + velY*velY)
				/ (LAUNCH_SPEED_MAX*LAUNCH_SPEED_MAX);
		u32 color = strength > 0.75 ? COLOR_DRED
				: strength > 0.5 ? COLOR_RED
				: strength > 0.25 ? COLOR_ORANGE
				: COLOR_LGREEN;
		plotTrajectoryPoint(projX, projY, velX, velY, 5, 3, 1, color);
		plotTrajectoryPoint(projX, projY, velX, velY, 10, 3, 1, color);
		plotTrajectoryPoint(projX, projY, velX, velY, 15, 3, 1, color);
		plotTrajectoryPoint(projX, projY, velX, velY, 20, 3, 1, color);
	}

	BG_Draw(bg, 0, 0, -1, 1, 1);
	Projectile_Draw(1);

	C2D_ViewReset();


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_WHITE);
	C2D_SceneBegin(top);

	BG_DrawFit(bg, 0, 0, 0, 400, 240);

	C2D_DrawText(&infoText->text, 0, 10, 20, 0, 0.5, 0.5);
}

static void sceneExit() {
	if (bg) BG_Free(bg);
	if (infoText) Text_Free(infoText);
	if (terrain) free(terrain);
}

Scene sceneCourse = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

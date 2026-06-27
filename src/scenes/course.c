#include <3ds.h>
#include <citro2d.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include "../scene.h"
#include "scene_internal.h"
#include "course.h"
#include "title.h"
#include "levelselector.h"
#include "error.h"
#include "results.h"
#include "components/text.h"
#include "components/background.h"
#include "components/border.h"
#include "../projectile.h"
#include "../projectiles/bomb.h"
#include "../rendering/colors.h"
#include "../rendering/rendertarget.h"
#include "../rendering/animation.h"
#include "../rendering/animations/firework.h"
#include "../util/touchinput.h"
#include "../util/macros.h"
#include "../terrain.h"
#include "../levelio.h"

#define LAUNCH_SPEED_MAX 6
#define TOUCHSCREEN_TO_LAUNCH_VEL_FACTOR 0.05

#define TEXT_MARGIN 10
#define LEVEL_NAME_Y 20
#define PAR_Y 15
#define LEVEL_PREVIEW_X 10
#define LEVEL_PREVIEW_Y (PAR_Y + 50)
#define LEVEL_PREVIEW_WIDTH 380
#define LEVEL_PREVIEW_HEIGHT (240 - 35 - LEVEL_PREVIEW_Y)

static int level;
static bool levelInRomfs;

static bool shouldFreeTerrain;
static int holeX, holeY, holeWidth, holeHeight;
static int fieldWidth;

static int strokes, par;
static bool hasFinished;
static Tracer projPath;
static bool shouldFreeProjPath;

static Text nameText, parText, strokesText;

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

Scene_Params Course_MakeParams(int level, bool inRomfs) {
	return (Scene_Params) { .course = {
		.level = level,
		.inRomfs = inRomfs
	} };
}

static bool sceneInit(Scene_Params params) {
	char *errMsg = "";  // Fill this in whenever you goto f_XYZ

	nameText = Text_Create(EDITOR_LEVEL_NAME_MAX + 1);
	if (!nameText) {
		errMsg = "Out of memory";
		goto f_nameText;
	}

	parText = Text_Create(9);
	if (!parText ) {
		errMsg = "Out of memory";
		goto f_parText ;
	}

	strokesText = Text_Create(13);
	if (!strokesText) {
		errMsg = "Out of memory";
		goto f_strokesText;
	}

	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(params.course.level, params.course.inRomfs, path);

	LevelIO_Hole hole;
	LevelIO_Proj proj;
	Tile (*tiles)[LEVEL_HEIGHT_TILES];
	Tile_WithPos *overlayTiles;
	size_t numOverlayTiles;
	char *name;
	if (!LevelIO_Read(path, &hole, &proj, &tiles, &overlayTiles,
			&numOverlayTiles, &fieldWidth, &par, &name)) {
		errMsg = "Level file is malformed or doesn't exist";
		goto f_LevelIORead;
	}
	Text_SetContent(nameText, name);
	Text_SetContent(parText, "Par %i", par);
	free(name);

	if (!Terrain_Init(fieldWidth, LEVEL_HEIGHT)) {
		errMsg = "Out of memory";
		goto f_Terrain;
	}
	shouldFreeTerrain = true;

	projPath = Tracer_Create(fieldWidth, LEVEL_HEIGHT);
	if (!projPath) {
		errMsg = "Out of memory";
		goto f_projPath;
	}
	shouldFreeProjPath = true;

	for (int x = 0; x < fieldWidth / TILE_SIZE; x++) {
		for (int y = 0; y < LEVEL_HEIGHT / TILE_SIZE; y++)  {
			Terrain_FillTile(x*TILE_SIZE, y*TILE_SIZE, tiles[x][y],
					false);
		}
	}
	free(tiles);
	for (size_t i = 0; i < numOverlayTiles; i++) {
		int x, y;
		Tile_GetPos(overlayTiles[i], &x, &y);
		Terrain_FillTile(x, y, overlayTiles[i], false);
	}

	holeX = hole.x;
	holeY = hole.y;
	holeWidth = hole.width;
	holeHeight = hole.height;

	Projectile_SetType(proj.type);
	Projectile_SetPos(proj.startX, proj.startY);
	Projectile_Reset();

	strokes  = 0;
	hasFinished = false;
	level = params.course.level;
	levelInRomfs = params.course.inRomfs;

	return true;

f_projPath:
	Terrain_Exit();
f_Terrain:
	free(tiles);
	free(overlayTiles);
f_LevelIORead:
	Text_Free(strokesText);
f_strokesText:
	Text_Free(parText);
f_parText:
	Text_Free(nameText);
f_nameText:
	Scene_SetNext(sceneError, Error_MakeParams(errMsg));
	return false;
}

static void sceneExit() {
	if (shouldFreeProjPath) Tracer_Free(projPath);
	if (shouldFreeTerrain) Terrain_Exit();
	Text_Free(strokesText);
	Text_Free(parText);
	Text_Free(nameText);
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
		strokes++;
	}
}

static void nextLevel() {
	shouldFreeTerrain = false;
	shouldFreeProjPath = false;
	Scene_SetNext(sceneResults, Results_MakeParams(strokes, level,
			levelInRomfs, projPath));
}

static void sceneUpdate() {
	u32 kDown = hidKeysDown();
//	u32 kHeld = hidKeysHeld();

	if (kDown & KEY_B) {
		if (levelInRomfs) {
			Scene_SetNext(sceneTitle, Title_MakeParams());
		} else {
			Scene_SetNext(sceneLevelSelector,
					LevelSelector_MakeParams(level));
		}
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
		if (!Animation_Start(animationFirework,
				Firework_MakeParams(x, y, strokes < par),
				nextLevel)) {
			nextLevel();
		}
	}
	Tracer_AddPoint(projPath, x, y);

	Terrain_Update();
	Projectile_Update();

	Text_SetContent(strokesText, "Strokes %i", strokes );
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
	Terrain_UpdateGraphics();


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	Text_Draw(nameText, TEXT_MARGIN, LEVEL_NAME_Y, 0, COLOR_DGREEN, 1,
			TEXT_LEFT);
	Text_Draw(parText, 400 - TEXT_MARGIN, PAR_Y, 0, COLOR_DGREEN, 1,
			TEXT_RIGHT);
	Text_Draw(strokesText, 400 - TEXT_MARGIN, PAR_Y + TEXT_LINE_HEIGHT,
			0, COLOR_DGREEN, 1, TEXT_RIGHT);
	int terrainX, terrainY, terrainWidth, terrainHeight;
	Terrain_Draw(LEVEL_PREVIEW_X, LEVEL_PREVIEW_Y, 0, LEVEL_PREVIEW_WIDTH,
			LEVEL_PREVIEW_HEIGHT, &terrainX, &terrainY, &terrainWidth,
			&terrainHeight);
	Border_Draw(terrainX, terrainY, 0, terrainWidth, terrainHeight);

	float projX, projY;
	Projectile_GetPos(&projX, &projY);
	C2D_DrawRectSolid(terrainX + (projX * terrainWidth) / fieldWidth,
			terrainY + (projY * terrainHeight) / LEVEL_HEIGHT,
			1, 2, 2, COLOR_WHITE);


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

	Terrain_Draw(0, 0, 0, fieldWidth, LEVEL_HEIGHT, NULL, NULL, NULL, NULL);
	Projectile_Draw(1);

	C2D_ViewReset();
}

Scene sceneCourse = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

#include <stdbool.h>
#include <malloc.h>
#include <alloca.h>
#include <3ds.h>
#include <citro2d.h>
#include "../scene.h"
#include "scene_internal.h"
#include "editor.h"
#include "levelselector.h"
#include "error.h"
#include "components/text.h"
#include "components/tileselector.h"
#include "components/background.h"
#include "components/border.h"
#include "components/button.h"
#include "components/editormenu.h"
#include "components/brushselector.h"
#include "components/popup.h"
#include "components/obstacleeditor.h"
#include "../rendering/rendertarget.h"
#include "../rendering/colors.h"
#include "../rendering/spritesheet.h"
#include "../rendering/animation.h"
#include "../environment/obstacle.h"
#include "../audio/music.h"
#include "../projectiles/bomb.h"
#include "../util/touchinput.h"
#include "../util/macros.h"
#include "../util/list.h"
#include "../tile.h"
#include "../levelio.h"

#define HOLE_WIDTH (TILE_SIZE * 2)
#define HOLE_HEIGHT (TILE_SIZE * 4)

#define SCROLL_UNIT TILE_SIZE

#define TEXT_MARGIN 10
#define LEVEL_NAME_Y 15
#define LEVEL_PREVIEW_X 10
#define LEVEL_PREVIEW_Y (LEVEL_NAME_Y + 35) 
#define LEVEL_PREVIEW_WIDTH 380
#define LEVEL_PREVIEW_HEIGHT 90
#define CONTROLS_TEXT_Y (LEVEL_PREVIEW_Y + LEVEL_PREVIEW_HEIGHT + 15)

static Background bg;
static float scroll;

static Tile (*tiles)[LEVEL_HEIGHT_TILES];
static Tile_WithPos (*overlayTiles)[LEVEL_HEIGHT_TILES];

static List obstacleList;
static Obstacle_Data *curObst;
static int curObstPoint;

static int holeX, holeY;
static int projX, projY;
static int par;
static int level;
static char *name;

static Text nameText, parText;
static Dispatcher touchDispatcher;

static Text infoText;
static int infoTextPage;

Scene_Params Editor_MakeParams(unsigned int level) {
	return (Scene_Params) { .editor = {
		.level = level
	} };
}

// Declarations needed to register with dispatcher, buttons
static bool handleTouchInput();
static void editName();
static void showExitPopup();
static void changePar(int change);

// Declarations for Obstacle_Data manipulation
static void freeObstacle(void *elem);
static bool addObstacle(LevelIO_Obst data, Obstacle_Data **newObst);
static bool getObstacles(LevelIO_Obst **obsts, size_t *numObsts);

static void pageInfoText();

static bool sceneInit(Scene_Params params) {
	bg = BG_Create(LEVEL_MAX_WIDTH, LEVEL_HEIGHT, COLOR_BLUE);
	if (!bg) goto f_bg;

	nameText = Text_Create(EDITOR_LEVEL_NAME_MAX + 1);
	if (!nameText) goto f_nameText;

	parText = Text_Create(9);
	if (!parText) goto f_parText;

	infoText = Text_Create(128);
	if (!infoText) goto f_infoText;
	infoTextPage = -1;
	pageInfoText();

	overlayTiles = calloc(LEVEL_MAX_WIDTH_TILES, sizeof(*overlayTiles));
	if (!overlayTiles) goto f_overlayTiles;

	obstacleList = List_Create();
	if (!obstacleList) goto f_obstacleList;

	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(params.editor.level, false, path);
	LevelIO_Hole hole;
	LevelIO_Proj proj;
	int width;
	Tile_WithPos *denseOverlayTiles;
	size_t numOverlayTiles;
	LevelIO_Obst *obstacles;
	size_t numObsts;
	if (LevelIO_Read(path, &hole, &proj, &tiles, &denseOverlayTiles,
			&numOverlayTiles, &obstacles, &numObsts, &width, &par,
			&name)) {
		Tile (*newTiles)[LEVEL_HEIGHT_TILES] = realloc(tiles,
				sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!newTiles) goto f_newTiles;
		tiles = newTiles;

		for (int x = width / TILE_SIZE; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				tiles[x][y] = Tile_Make(SPRITE_TILE_SKY, 0);
			}
		}

		holeX = hole.x;
		holeY = hole.y;
		projX = proj.startX;
		projY = proj.startY;

		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				BG_DrawTile(bg, tiles[x][y], x * TILE_SIZE,
						y * TILE_SIZE, false);
			}
		}

		for (size_t i = 0; i < numOverlayTiles; i++) {
			Tile_WithPos overlayTile = denseOverlayTiles[i];
			int x, y;
			Tile_GetPos(denseOverlayTiles[i], &x, &y);
			overlayTiles[x/TILE_SIZE][y/TILE_SIZE] = overlayTile;
			BG_DrawTile(bg, overlayTile, x, y, false);
		}
		free(denseOverlayTiles);

		for (size_t i = 0; i < numObsts; i++) {
			addObstacle(obstacles[i], NULL);
		}
		free(obstacles);
	} else {
		tiles = malloc(sizeof(*tiles) * LEVEL_MAX_WIDTH_TILES);
		if (!tiles) goto f_tiles;

		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
				tiles[x][y] = Tile_Make(SPRITE_TILE_SKY, 0);
			}
		}

		holeX = holeY = 0;
		projX = 40 + (TILE_SIZE / 2);
		projY = 190 + (TILE_SIZE / 2);
		name = malloc(sizeof('\0'));
		name[0] = '\0';
	}
	Text_SetContent(nameText, name);

	if (!TileSelector_Init(Tile_Make(SPRITE_TILE_GRASS, 0))) goto f_TileSelector;

	touchDispatcher = Dispatcher_Create();
	if (!touchDispatcher) goto f_touchDispatcher;
	Dispatcher_AddHandler(touchDispatcher, (Dispatcher_Handler) {
			.priority = 0, NULL, handleTouchInput });
	TileSelector_RegisterForTouchEvents(touchDispatcher, 2);

	if (!EditorMenu_Init(editName, showExitPopup, changePar)) goto f_EditorMenu;
	EditorMenu_RegisterForTouchEvents(touchDispatcher, 1);

	if (!BrushSelector_Init(BRUSH_PENCIL)) goto f_BrushSelector;
	BrushSelector_RegisterForTouchEvents(touchDispatcher, 1);

	if (!ObstacleEditor_Init()) goto f_ObstacleEditor;
	ObstacleEditor_RegisterForTouchEvents(touchDispatcher, 3);

	Music_Start(MUSIC_EDITOR);

	infoTextPage = 0;
	scroll = 0;
	level = params.editor.level;
	curObst = NULL;
	curObstPoint = 0;

	return true;

f_ObstacleEditor:
	BrushSelector_Exit();
f_BrushSelector:
	EditorMenu_Exit();
f_EditorMenu:
	Dispatcher_Free(touchDispatcher);
f_touchDispatcher:
	TileSelector_Exit();
f_TileSelector:
f_newTiles:
	free(tiles);
f_tiles:
	List_Free(obstacleList);
f_obstacleList:
	free(overlayTiles);
f_overlayTiles:
	Text_Free(infoText);
f_infoText:
	Text_Free(parText);
f_parText:
	Text_Free(nameText);
f_nameText:
	BG_Free(bg);
f_bg:
	Scene_SetNext(sceneError, Error_MakeParams("Out of memory"));
	return false;
}

static void sceneExit() {
	BG_Free(bg);
	free(tiles);
	free(overlayTiles);
	List_ForEach(obstacleList, freeObstacle);
	List_Free(obstacleList);
	free(name);
	Text_Free(nameText);
	Text_Free(parText);
	Text_Free(infoText);
	Dispatcher_Free(touchDispatcher);
	TileSelector_Exit();
	EditorMenu_Exit();
	BrushSelector_Exit();
	ObstacleEditor_Exit();
	Music_Stop();
}

static bool exportLevel() {
	char path[LEVEL_PATH_MAX];
	LevelIO_MakePath(level, false, path);

	LevelIO_Hole hole = { holeX, holeY, HOLE_WIDTH, HOLE_HEIGHT };
	LevelIO_Proj proj = { projX, projY, projectileBomb };

	int tilesMaxX = 0;
	size_t numOverlayTiles = 0;
	for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			if (overlayTiles[x][y] != 0) {
				numOverlayTiles++;
			}
			if (Tile_GetSprite(tiles[x][y]) != SPRITE_TILE_SKY
					&& x > tilesMaxX) {
				tilesMaxX = x;
			}
		}
	}

	Tile_WithPos *denseOverlayTiles = malloc(sizeof(*denseOverlayTiles)
			* numOverlayTiles);
	if (!denseOverlayTiles) goto f_denseOverlayTiles;
	size_t i = 0;
	for (int y = 0; y < LEVEL_HEIGHT_TILES; y++) {
		for (int x = 0; x < LEVEL_MAX_WIDTH_TILES; x++) {
			if (overlayTiles[x][y] != 0) {
				denseOverlayTiles[i] = overlayTiles[x][y];
				i++;
			}	
		}
	}

	LevelIO_Obst *obstacles;
	size_t numObsts;
	if (!getObstacles(&obstacles, &numObsts)) goto f_obstacles;

	bool success = LevelIO_Write(path, hole, proj, tiles,
			denseOverlayTiles, numOverlayTiles,
			obstacles, numObsts,
			(tilesMaxX + 1) * TILE_SIZE, par, name);

	free(denseOverlayTiles);
	free(obstacles);
	return success;

f_obstacles:
	free(denseOverlayTiles);
f_denseOverlayTiles:
	return false;
}

static void changeTile(int tileX, int tileY, Tile newTile) {
	if (Tile_IsOverlay(newTile)) {
		overlayTiles[tileX][tileY] = Tile_AddPos(newTile, tileX * TILE_SIZE,
				tileY * TILE_SIZE);
		BG_DrawTile(bg, tiles[tileX][tileY], tileX * TILE_SIZE,
				tileY * TILE_SIZE, true);
		BG_DrawTile(bg, newTile, tileX * TILE_SIZE, tileY * TILE_SIZE,
				false);
	} else {
		tiles[tileX][tileY] = newTile;
		overlayTiles[tileX][tileY] = 0;
		BG_DrawTile(bg, newTile, tileX * TILE_SIZE, tileY * TILE_SIZE, true);
	}
}

// Sets curObst and curObstPoint with the obstacle over argTileX, argTileY
static void findObstacle(int argTileX, int argTileY) {
	// Make these static so 3DS doesn't crash when they're used in check
	static int tileX, tileY;
	tileX = argTileX, tileY = argTileY;

	void check(void *elem) {
		Obstacle_Data *obst = (Obstacle_Data*)elem;
		for (int i = 0; i < obst->numPoints; i++) {
			if (obst->xs[i] / TILE_SIZE == tileX
					&& obst->ys[i] / TILE_SIZE == tileY) {
				curObst = obst;
				curObstPoint = i;
				return;
			}
		}
	}

	curObst = NULL;
	curObstPoint = 0;
	List_ForEach(obstacleList, check);
}

static void removePoint(Obstacle_Data *data, int point) {
	// Make this static so 3DS doesn't crash when it's used in test
	static Obstacle_Data *toRemove;
	bool test(void *elem) {
		return elem == toRemove;
	}

	for (int i = point; i < data->numPoints - 1; i++) {
		data->xs[i] = data->xs[i+1];
		data->ys[i] = data->ys[i+1];
	}
	data->numPoints--;
	if (data->numPoints == 0) {
		toRemove = data;
		List_Filter(obstacleList, test, freeObstacle);
	} else {
		int *newXs = realloc(data->xs, sizeof(*data->xs) * data->numPoints);
		if (newXs) data->xs = newXs;
		int *newYs = realloc(data->ys, sizeof(*data->ys) * data->numPoints);
		if (newYs) data->ys = newYs;
	} 
}

static void addPoint(Obstacle_Data *data, int before, int x, int y) {
	data->numPoints++;

	int *newXs = realloc(data->xs, sizeof(*data->xs) * data->numPoints);
	if (!newXs) return;
	data->xs = newXs;
	int *newYs = realloc(data->ys, sizeof(*data->ys) * data->numPoints);
	if (!newYs) return;
	data->ys = newYs;

	for (int i = data->numPoints - 1; i > before; i--) {
		data->xs[i] = data->xs[i-1];
		data->ys[i] = data->ys[i-1];
	}
	data->xs[before] = x;
	data->ys[before] = y;
}

static bool handleTouchInput() {
	if (!TouchInput_InProgress()) return false;

	float courseX = TouchInput_GetSwipe().end.px + scroll;
	float courseY = TouchInput_GetSwipe().end.py;
	int tileX = courseX / TILE_SIZE;
	int tileY = courseY / TILE_SIZE;

	switch (BrushSelector_GetBrush()) {
		case BRUSH_PENCIL:
			changeTile(tileX, tileY, TileSelector_GetTile());
			break;
		case BRUSH_RECTANGLE:
			int tileX2 = (TouchInput_GetSwipe().start.px + scroll)
					/ TILE_SIZE;
			int tileY2 = (TouchInput_GetSwipe().start.py)
					/ TILE_SIZE;
			int startX = tileX > tileX2 ? tileX2 : tileX;
			int endX = tileX > tileX2 ? tileX : tileX2;
			int startY = tileY > tileY2 ? tileY2 : tileY;
			int endY = tileY > tileY2 ? tileY : tileY2;

			for (int x = startX; x <= endX; x++) {
				for (int y = startY; y <= endY; y++) {
					changeTile(x, y, TileSelector_GetTile());
				}
			}
			break;
		case BRUSH_BALL_POS:
			projX = tileX * TILE_SIZE + (TILE_SIZE / 2);
			projY = tileY * TILE_SIZE + (TILE_SIZE / 2);
			break;
		case BRUSH_HOLE_POS:
			holeX = tileX * TILE_SIZE;
			holeY = tileY * TILE_SIZE;
			break;
		case BRUSH_OBSTACLE_ADD:
			Obstacle_Data *new;
			addObstacle(
					(LevelIO_Obst) {
						0, 1,
						(int[]) { tileX * TILE_SIZE
							+ TILE_SIZE/2 },
						(int[]) { tileY * TILE_SIZE
							+ TILE_SIZE/2 },
						1, 1
					},
					&new
				);
			ObstacleEditor_Show(new);
			break;
		case BRUSH_OBSTACLE_DEL:
			findObstacle(tileX, tileY);
			if (curObst) removePoint(curObst, curObstPoint);
			break;
		case BRUSH_OBSTACLE_EDIT:
			findObstacle(tileX, tileY);
			if (curObst) ObstacleEditor_Show(curObst);
			break;
		case BRUSH_OBSTACLE_MOVE:
			if (!curObst) findObstacle(tileX, tileY);
			if (curObst) {
				curObst->xs[curObstPoint] = tileX * TILE_SIZE
						+ TILE_SIZE/2;
				curObst->ys[curObstPoint] = tileY * TILE_SIZE
						+ TILE_SIZE/2;
			}
			break;
		case BRUSH_OBSTACLE_DUPE:
			findObstacle(tileX, tileY);
			if (curObst) {
				addPoint(curObst, curObstPoint,
						tileX * TILE_SIZE + TILE_SIZE/2,
						tileY * TILE_SIZE + TILE_SIZE/2);
				BrushSelector_SetBrush(BRUSH_OBSTACLE_MOVE);
			}
			break;
		case NUM_BRUSHES: break;  // Satisfy the compiler
	}

	return true;
}

static void editName() {
	SwkbdState keyboard;
	SwkbdButton pressedButton;
	char buf[EDITOR_LEVEL_NAME_MAX + 1];
	swkbdInit(&keyboard, SWKBD_TYPE_QWERTY, 2, EDITOR_LEVEL_NAME_MAX);
	swkbdSetValidation(&keyboard, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
	swkbdSetHintText(&keyboard, "Enter a name");
	swkbdSetInitialText(&keyboard, name);

	pressedButton = swkbdInputText(&keyboard, buf, EDITOR_LEVEL_NAME_MAX + 1);
	if (pressedButton != SWKBD_BUTTON_CONFIRM) {
		return;
	}

	char *newName = realloc(name, sizeof(char) * (strlen(buf) + 1));
	if (!newName) return;
	name = newName;
	strcpy(name, buf);
	Text_SetContent(nameText, name);
}

static void saveExit() {
	Popup_Exit();
	if (exportLevel()) {
		Scene_SetNext(sceneLevelSelector, LevelSelector_MakeParams(level));
	} else {
		Popup_Init("Failed to save file", POPUP_ONE_BUTTON,
				(Popup_Button[]) { { "OK", -1, NULL, Popup_Exit } });
	}
}

static void exitNoSave() {
	Scene_SetNext(sceneLevelSelector, LevelSelector_MakeParams(level));
	Popup_Exit();
}

static void showExitPopup() {
	Popup_Button buttons[] = {
			{ "Don't Save", KEY_B, NULL, exitNoSave },
			{ "Save & Exit", KEY_A, NULL, saveExit }
		};
	Popup_Init("Save before exiting?", POPUP_TWO_BUTTON, buttons);
}

static void changePar(int change) {
	par += change;
}

static bool addObstacle(LevelIO_Obst data, Obstacle_Data **newObst) {
	if (!newObst) newObst = alloca(sizeof(*newObst));

	*newObst = malloc(sizeof(**newObst));
	if (!(*newObst)) goto f_newObst;

	(*newObst)->xs = malloc(sizeof(*(*newObst)->xs) * data.numPoints);
	if (!(*newObst)->xs) goto f_xs;

	(*newObst)->ys = malloc(sizeof(*(*newObst)->ys) * data.numPoints);
	if (!(*newObst)->ys) goto f_ys;

	(*newObst)->sprite1 = data.sprite1;
	(*newObst)->sprite2 = data.sprite2;
	for (int i = 0; i < data.numPoints; i++) {
		(*newObst)->xs[i] = data.xs[i];
		(*newObst)->ys[i] = data.ys[i];
	}
	(*newObst)->numPoints = data.numPoints;
	(*newObst)->speed = data.speed;

	if (!List_Push(obstacleList, *newObst)) goto f_List_Push;

	return true;

f_List_Push:
	free((*newObst)->ys);
f_ys:
	free((*newObst)->xs);
f_xs:
	free(*newObst);
f_newObst:
	return false;
}

// Signature designed for List operations
static void freeObstacle(void *elem) {
	Obstacle_Data *obst = (Obstacle_Data*)elem;
	free(obst->xs);
	free(obst->ys);
	free(obst);
}

static bool getObstacles(LevelIO_Obst **obsts, size_t *numObsts) {
	// Make these static so 3DS doesn't crash when they're used by putObst
	static size_t i;
	static LevelIO_Obst **obstsCopy;

	void putObst(void *elem) {
		Obstacle_Data *obst = (Obstacle_Data*)elem;
		(*obstsCopy)[i].sprite1 = obst->sprite1;
		(*obstsCopy)[i].sprite2 = obst->sprite2;
		(*obstsCopy)[i].xs = obst->xs;
		(*obstsCopy)[i].ys = obst->ys;
		(*obstsCopy)[i].numPoints = obst->numPoints;
		(*obstsCopy)[i].speed = obst->speed;
		i++;
	}

	*numObsts = List_Length(obstacleList);
	*obsts = malloc(sizeof(**obsts) * *numObsts);
	if (!(*obsts)) return false;

	i = 0;
	obstsCopy = obsts;
	List_ForEach(obstacleList, putObst);
	return true;
}

static void pageInfoText() {
	infoTextPage = (infoTextPage + 1) % 4;
	switch (infoTextPage) {
		case 0:
			Text_SetContent(infoText, "\n\n\n%c : Show button help",
					TEXT_KEY_X);
			break;
		case 1:
			Text_SetContent(infoText, "   : Place individual tiles\n"
					"   : Fill area with tiles\n"
					"   : Move ball starting location\n"
					"%c : Next page", TEXT_KEY_X);
			break;
		case 2:
			Text_SetContent(infoText, "   : Move hole location\n"
					"   : Place new moving obstacle\n"
					"   : Delete obstacle point\n"
					"%c : Next page", TEXT_KEY_X);
			break;
		case 3:
			Text_SetContent(infoText, "   : Edit obstacle properties\n"
					"   : Move obstacle point\n"
					"   : Duplicate obstacle point\n"
					"%c : Next page", TEXT_KEY_X);
			break;
	}
}

static void drawInfoText(float x, float y, float depth) {
	Text_Draw(infoText, x, y, 0, COLOR_DGRAY, 1, TEXT_LEFT);
	y += 3;
	x -= 2;
	switch (infoTextPage) {
		case 0:
			break;
		case 1:
			SpriteSheet_Draw(SPRITE_PENCIL_BUTTON, x, y, depth, 0,
					false, false);
			y += TEXT_LINE_HEIGHT;
			SpriteSheet_Draw(SPRITE_RECTANGLE_BUTTON, x, y, depth, 0,
					false, false);
			y += TEXT_LINE_HEIGHT;
			SpriteSheet_Draw(SPRITE_BALL_BUTTON, x, y, depth, 0,
					false, false);
			break;
		case 2:
			SpriteSheet_Draw(SPRITE_HOLE_BUTTON, x, y, depth, 0,
					false, false);
			y += TEXT_LINE_HEIGHT;
			SpriteSheet_Draw(SPRITE_BIRD_BUTTON, x, y, depth, 0,
					false, false);
			y += TEXT_LINE_HEIGHT;
			SpriteSheet_Draw(SPRITE_X_BUTTON, x, y, depth, 0,
					false, false);
			break;
		case 3:
			SpriteSheet_Draw(SPRITE_WRENCH_BUTTON, x, y, depth, 0,
					false, false);
			y += TEXT_LINE_HEIGHT;
			SpriteSheet_Draw(SPRITE_HAND_BUTTON, x, y, depth, 0,
					false, false);
			y += TEXT_LINE_HEIGHT;
			SpriteSheet_Draw(SPRITE_DUPE_BUTTON, x, y, depth, 0,
					false, false);
			break;
	}
}

static void sceneUpdate(float _) {
	if (BG_IsUpdating(bg)) return;

	u32 kDown = hidKeysDown();
	u32 kHeld = hidKeysHeld();

	if (kDown & KEY_B) showExitPopup();

	if (kHeld & KEY_CPAD_LEFT || kHeld & KEY_CSTICK_LEFT)
		scroll -= SCROLL_UNIT;
	if (kHeld & KEY_CPAD_RIGHT || kHeld & KEY_CSTICK_RIGHT)
		scroll += SCROLL_UNIT;
	scroll = clamp(scroll, 0, LEVEL_MAX_WIDTH - 320);

	if (kDown & KEY_X) pageInfoText();

	if (TouchInput_JustFinished()) {
		curObst = NULL;
		curObstPoint = 0;
	}

	Dispatcher_DispatchEvent(touchDispatcher);

	Text_SetContent(parText, "Par %i", par);
}

static void drawRectOutline(int x, int y, int width, int height, u32 color, int 		outlineWidth) {
	C2D_DrawRectSolid(x, y, 0, width, outlineWidth, color);
	C2D_DrawRectSolid(x, y, 0, outlineWidth, height, color);
	C2D_DrawRectSolid(x, y + height - outlineWidth, 0, width, outlineWidth,
			color);
	C2D_DrawRectSolid(x + width - outlineWidth, y, 0, outlineWidth, height,
			color);
}

// Signature designed for List operations
static void drawObstacleTop(void *elem) {
	Obstacle_Data *obst = (Obstacle_Data*)elem;

	C3D_Mtx prevMtx;
	C2D_ViewSave(&prevMtx);
	C2D_ViewTranslate(LEVEL_PREVIEW_X, LEVEL_PREVIEW_Y);
	C2D_ViewScale((float)LEVEL_PREVIEW_WIDTH / LEVEL_MAX_WIDTH,
			(float)LEVEL_PREVIEW_HEIGHT / LEVEL_HEIGHT);

	int firstX = obst->xs[0];
	int firstY = obst->ys[0];
	int secondX = obst->numPoints > 1 ? obst->xs[1] : obst->xs[0];
	int secondY = obst->numPoints > 1 ? obst->ys[1] : obst->ys[0];

	SpriteSheet_DrawObstacle(
			obst->sprite1,
			firstX,
			firstY,
			0.5,
			firstX == secondX && firstY != secondY ? M_PI/2 : 0,
			firstX > secondX || (firstX == secondX && firstY > secondY),
			firstX == secondX && firstY != secondY
		);

	C2D_ViewRestore(&prevMtx);
}

// Signature designed for List operations
static void drawObstacleBottom(void *elem) {
	Obstacle_Data *obst = (Obstacle_Data*)elem;
	for (int i = 0; i < obst->numPoints; i++) {
		int nextI = (i + 1) % obst->numPoints;

		bool flipHoriz = obst->xs[i] > obst->xs[nextI]
				|| (obst->xs[i] == obst->xs[nextI]
					&& obst->ys[i] > obst->ys[nextI]);
		bool flipVert = obst->xs[i] == obst->xs[nextI]
				&& obst->ys[i] != obst->ys[nextI];
		bool rotate = obst->xs[i] == obst->xs[nextI]
				&& obst->ys[i] != obst->ys[nextI];

		C2D_DrawLine(obst->xs[i], obst->ys[i], COLOR_ORANGE,
				obst->xs[nextI], obst->ys[nextI], COLOR_ORANGE,
				1, -1);
		SpriteSheet_DrawObstacle(obst->sprite1, obst->xs[i], obst->ys[i],
				-0.9, rotate ? M_PI/2 : 0, flipHoriz, flipVert);
	}
	SpriteSheet_Draw(SPRITE_SMALL_ONE, obst->xs[0], obst->ys[0], -0.8, 0, false,
			false);
}

static void sceneDraw() {
	BG_UpdateGraphics(bg);
	TileSelector_UpdateGraphics();


	C3D_RenderTarget *top = RenderTarget_GetTop();
	C2D_TargetClear(top, COLOR_LGRAY);
	C2D_SceneBegin(top);

	Text_Draw(nameText, TEXT_MARGIN, LEVEL_NAME_Y, 0, COLOR_DGREEN, 1,
			TEXT_LEFT);
	Text_Draw(parText, 390, LEVEL_NAME_Y, 0, COLOR_DGREEN, 1, TEXT_RIGHT);
	int bgX, bgY, bgWidth, bgHeight;
	BG_DrawFit(bg, LEVEL_PREVIEW_X, LEVEL_PREVIEW_Y, 0, LEVEL_PREVIEW_WIDTH,
			LEVEL_PREVIEW_HEIGHT, &bgX, &bgY, &bgWidth, &bgHeight);
	Border_Draw(bgX, bgY, 0, bgWidth, bgHeight);
	List_ForEach(obstacleList, drawObstacleTop);
	drawInfoText(TEXT_MARGIN,
			LEVEL_PREVIEW_Y + LEVEL_PREVIEW_HEIGHT + TEXT_MARGIN,
			1);


	C3D_RenderTarget *bottom = RenderTarget_GetBottom();
	C2D_TargetClear(bottom, COLOR_WHITE);
	C2D_SceneBegin(bottom);

	C2D_ViewTranslate(-scroll, 0);

	BG_Draw(bg, 0, 0, -1, 1, 1);
	drawRectOutline(holeX, holeY, HOLE_WIDTH, HOLE_HEIGHT, COLOR_DRED, 2);
	Animation_Draw(0.5);
	SpriteSheet_DrawCentered(SPRITE_BALL, projX, projY, 0.5, 0, false, false);
	List_ForEach(obstacleList, drawObstacleBottom);

	C2D_ViewReset();

	BrushSelector_Draw(0.4);
	EditorMenu_Draw(0.4);
	TileSelector_Draw(0.5);
	ObstacleEditor_Draw(1);
}

Scene sceneEditor = &(struct scene) {
	.init = sceneInit,
	.update = sceneUpdate,
	.draw = sceneDraw,
	.exit = sceneExit
};

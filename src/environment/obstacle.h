/*
 * Part of the environment which moves in a set path. A path is a looping set
 * of points which the obstacle passes through.
 */

#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <stdbool.h>
#include "../rendering/spritesheet.h"

typedef struct {
	/*
	 * Cycles through these two sprites while moving. Both sprites should
	 * be of the same size. Hitbox is deduced from sprite1.
	 */
	SpriteSheet_ObstSprite sprite1;
	SpriteSheet_ObstSprite sprite2;
	/*
	 * Path defined by pairs from xs and ys, both of length numPoints; when
	 * the last point is reached, cycles back to the first.
	 */
	int *xs;
	int *ys;
	int numPoints;
	/*
	 * In pixels/frame.
	 */
	float speed;
} Obstacle_Data;

/*
 * Call this before anything else.
 */
bool Obstacle_Init();

void Obstacle_Exit();

/*
 * Creates an obstacle and adds it to the environment.
 */
bool Obstacle_Add(Obstacle_Data data);

/*
 * Removes any obstacles which overlap (x, y).
 */
void Obstacle_Destroy(int x, int y);

/*
 * Removes any obstacles which overlap the circle centered at (x, y) and with
 * radius radius.
 */
void Obstacle_DestroyCircle(int x, int y, int radius);

/*
 * Removes all obstacles.
 */
void Obstacle_Clear();

/*
 * Returns true if there is an obstacle at the point (x, y).
 */
bool Obstacle_IsAt(int x, int y);

/*
 * Moves all obstacles and updates their animations.
 */
void Obstacle_Update();

/*
 * Draws all obstacles at their locations in the environment.
 */
void Obstacle_Draw(float depth);

#endif

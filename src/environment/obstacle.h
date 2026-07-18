/*
 * Part of the environment which moves in a set path. A path is a looping set
 * of points which the obstacle passes through.
 */

#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <stdbool.h>
#include "../rendering/spritesheet.h"

/*
 * Call this before anything else.
 */
bool Obstacle_Init();

void Obstacle_Exit();

/*
 * Creates an obstacle and adds it to the environment. The obstacle will cycle
 * through sprite1 and sprite2 when moving.
 *
 * sprite1 and sprite2 should have the same width and height.
 *
 * The path is defined by pairs from xs and ys starting with (xs[0], ys[0]);
 * each array should be of length numPoints. When
 * (xs[numPoints-1], ys[numPoints-1]) is reached, the path is automatically
 * connected path back to the start.
 *
 * The obstacle will move at speed pixels/frame.
 */
bool Obstacle_Add(SpriteSheet_ObstSprite sprite1, SpriteSheet_ObstSprite sprite2,
		int xs[], int ys[], int numPoints, float speed);

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

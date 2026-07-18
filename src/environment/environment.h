/*
 * Provides wrapper functions to check/effect the terrain and obstacle systems
 * at once.
 */

#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "terrain.h"

/*
 * Removes all terrain and obstacles from the circle of radius radius centered
 * at (x, y).
 */
void Env_ClearCircle(int x, int y, int radius);

/*
 * Like Terrain_TypeAt except obstacles count as TERRAIN_GROUND.
 */
Terrain_Type Env_TypeAt(int x, int y);

#endif

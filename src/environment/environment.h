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
 * Like Env_ClearCircle except terrain of type excluded is not removed.
 */
void Env_ClearCircleExcept(int x, int y, int radius, Terrain_Type excluded);

/*
 * Like Terrain_TypeAt except obstacles count as TERRAIN_GROUND.
 */
Terrain_Type Env_TypeAt(int x, int y);

#endif

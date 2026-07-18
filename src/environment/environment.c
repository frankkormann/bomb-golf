#include "environment.h"
#include "terrain.h"
#include "obstacle.h"
#include "../scenes/course.h"

void Env_ClearCircle(int x, int y, int radius) {
	//https://stackoverflow.com/a/24453110
	int r2 = radius * radius;
	int area = r2 << 2;
	int rr = radius << 1;

	for (int i = 0; i < area; i++) {
		int tx = (i % rr) - radius;
		int ty = (i / rr) - radius;
		int nx = x + tx;
		int ny = y + ty;
		if (tx * tx + ty * ty <= r2 && nx >= 0 && ny >= 0
				&& nx < Course_GetFieldWidth()
				&& ny < Course_GetFieldHeight()) {
			Terrain_ClearPixel(nx, ny);
		}
	}
	Obstacle_DestroyCircle(x, y, radius);
}

Terrain_Type Env_TypeAt(int x, int y) {
	Terrain_Type type = Terrain_TypeAt(x, y);
	if (type != TERRAIN_NOTHING) {
		return type;
	} else if (Obstacle_IsAt(x, y)) {
		return TERRAIN_GROUND;
	}
	return TERRAIN_NOTHING;
}

/*
 * Draws a set of points.
 */

#ifndef TRACER_H
#define TRACER_H

typedef struct tracer *Tracer;

/*
 * maxX and maxY define the bounds for Tracer_Draw and are not actual maximums.
 *
 * Returns a new Tracer or NULL if there isn't enough memory.
 */
Tracer Tracer_Create(float maxX, float maxY);

void Tracer_Free(Tracer tracer);

bool Tracer_AddPoint(Tracer tracer, float x, float y);

/*
 * Draws to tracer's internal graphics. Call this before Tracer_Draw.
 */
void Tracer_UpdateGraphics(Tracer tracer);

/*
 * Draws each point, scaled to fit them all within a rectangle whose top-left
 * corner is at (x, y) and dimensions (width, height).
 */
void Tracer_Draw(Tracer tracer, int x, int y, float depth, int width, int height);

#endif

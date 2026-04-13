/*
 * Simple gray border, essentially an unfilled rectangle.
 */

#ifndef BORDER_H
#define BORDER_H

#define BORDER_WIDTH 5

/*
 * Draws a border bounding a rectangle with its top-left corner at (x, y).
 * The space inside the border is specified by x, y, width, height.
 */
void Border_Draw(float x, float y, float depth, float width, float height);

#endif

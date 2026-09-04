/*
 * Code generation for stereoscopic 3D. Transforms code for drawing 2D images
 * to the top screen normally into stereoscopic 3D code.
 *
 * #define D3D_VALS with an array of pairs of { x, depth }. Each pair describes
 * the x-position and depth that a component should be drawn at.
 *
 * Then #define D3D_CODE with the code to draw on the top screen. The following
 * macros are provided for the code:
 *
 * - D3D_TARGET: The C3D_RenderTarget currently in use.
 * - D3D_X(i):   The x-position of the ith component, D3D_XS(i) corresponding
 *               to D3D_VALS[i].
 * - D3D_Xi(i):  Like above, except the correction is rounded up to an integer.
 * - D3D_D(i):   The depth of the ith component, D3D_DS(i) corresponding to
 *               D3D_VALS[i].
 * - D3D_CORRECTION(i): The correction applied to each x value, that is, how
 *                      much D3D_X differs from the value in D3D_VALS.
 *
 * If you want to use D3D_CORRECTION in D3D_VALS, instead you must split it by
 * defining D3D_DEPTHS and D3D_XS. Then D3D_CORRECTION can be used in D3D_XS
 * but not in D3D_DEPTHS.
 *
 * Any variables used in D3D_VALS, D3D_XS, or D3D_DEPTHS must be declared ahead
 * of time, although their values will not be read until required by an
 * instance of D3D_X/D3D_Xi or D3D_D.
 *
 * After #define'ing everything, #include draw3d_gen.h.
 * All macros described above will be #undef'd automatically.
 */

#ifndef DRAW3D_H
#define DRAW3D_H

#include <math.h>
#include "rendertarget.h"

#define D3D_3D_STRENGTH 3

typedef struct {
	int x;
	float depth;
} D3D_Value;

#endif

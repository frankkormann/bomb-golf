#ifndef DRAW3D_H
#error "Include draw3d.h"
#endif

#define D3D_CEIL(x) ((int)(x) == (x) ? (x) : (int)(x) + 1)

{
float slider = osGet3DSliderState();
int sign;

#ifdef D3D_VALS
#define D3D_D(i)		((D3D_Value[])D3D_VALS[i].depth)
#define D3D_CORRECTION(i)	D3D_CEIL(D3D_3D_STRENGTH * slider * D3D_D(i))
#define D3D_X(i)		((D3D_Value[])D3D_VALS[i].x + sign * D3D_CORRECTION(i))
#else
#define D3D_D(i)		((float[])D3D_DEPTHS[i])
#define D3D_CORRECTION(i)	D3D_CEIL(D3D_3D_STRENGTH * slider * D3D_D(i))
#define D3D_X(i)		((int[])D3D_XS[i] + sign * D3D_CORRECTION(i))
#endif

{
	C3D_RenderTarget *left = RenderTarget_Left();
	sign = 1;
	#define D3D_TARGET left
	D3D_CODE
	#undef D3D_TARGET
}

{
	C3D_RenderTarget *right = RenderTarget_Right();
	sign = -1;
	#define D3D_TARGET right
	D3D_CODE
	#undef D3D_TARGET
}

#undef D3D_VALS
#undef D3D_CODE
#undef D3D_D
#undef D3D_CORRECTION
#undef D3D_X
}

#undef D3D_CEIL

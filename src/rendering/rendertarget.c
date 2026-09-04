#include <3ds.h>
#include <citro2d.h>
#include "rendertarget.h"

C3D_RenderTarget *left, *right, *bottom;

C3D_RenderTarget* RenderTarget_Left() {
	if (!left) {
		left = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	}
	return left;
}

C3D_RenderTarget* RenderTarget_Right() {
	if (!right) {
		right = C2D_CreateScreenTarget(GFX_TOP, GFX_RIGHT);
	}
	return right;
}

C3D_RenderTarget* RenderTarget_Bottom() {
	if (!bottom) {
		bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	}
	return bottom;
}

void RenderTarget_DeleteAll() {
	if (left) C3D_RenderTargetDelete(left);
	if (right) C3D_RenderTargetDelete(right);
	if (bottom) C3D_RenderTargetDelete(bottom);
}
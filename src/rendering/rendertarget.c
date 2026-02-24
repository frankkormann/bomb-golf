#include <3ds.h>
#include <citro2d.h>
#include "rendertarget.h"

C3D_RenderTarget *top, *bottom;

C3D_RenderTarget* RenderTarget_GetTop() {
	if (!top) {
		top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
	}
	return top;
}

C3D_RenderTarget* RenderTarget_GetBottom() {
	if (!bottom) {
		bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
	}
	return bottom;
}

void RenderTarget_DeleteAll() {
	if (top) C3D_RenderTargetDelete(top);
	if (bottom) C3D_RenderTargetDelete(bottom);
}
/*
 * Ensures excess C3D_RenderTargets are not created. Use this instead of
 * C2D_CreateScreenTarget.
 */

#ifndef RENDERTARGET_H
#define RENDERTARGET_H

C3D_RenderTarget* RenderTarget_GetTop();
C3D_RenderTarget* RenderTarget_GetBottom();

void RenderTarget_DeleteAll();

#endif

#ifndef IRENDERABLE_H
#define IRENDERABLE_H

#include "ERenderOptions.h"
#include <Common/CAABox.h>
#include <Common/types.h>
#include <Core/CFrustumPlanes.h>

class CRenderer;

class IRenderable
{
public:
    IRenderable() {}
    virtual ~IRenderable() {}
    virtual void AddToRenderer(CRenderer *pRenderer, const CFrustumPlanes& frustum) = 0;
    virtual void Draw(ERenderOptions options) {}
    virtual void DrawAsset(ERenderOptions options, u32 asset) {}
    virtual void DrawSelection() {}
};

#endif // IRENDERABLE_H

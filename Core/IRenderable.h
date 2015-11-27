#ifndef IRENDERABLE_H
#define IRENDERABLE_H

#include "ERenderOptions.h"
#include "SViewInfo.h"
#include <Common/types.h>

class CRenderer;

class IRenderable
{
public:
    IRenderable() {}
    virtual ~IRenderable() {}
    virtual void AddToRenderer(CRenderer* pRenderer, const SViewInfo& ViewInfo) = 0;
    virtual void Draw(ERenderOptions /*Options*/, int /*ComponentIndex*/, const SViewInfo& /*ViewInfo*/) {}
    virtual void DrawSelection() {}
};

#endif // IRENDERABLE_H

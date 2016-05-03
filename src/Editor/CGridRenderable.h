#ifndef CGRIDRENDERABLE
#define CGRIDRENDERABLE

#include <Core/Render/CRenderer.h>
#include <Core/Render/IRenderable.h>

// Tiny helper to make sure the grid draws at the correct depth.
class CGridRenderable : public IRenderable
{
public:
    void AddToRenderer(CRenderer *pRenderer, const SViewInfo&)
    {
        pRenderer->AddMesh(this, 0, CAABox::skOne, false, eDrawMesh);
    }

    void Draw(FRenderOptions, int, ERenderCommand, const SViewInfo&)
    {
        CDrawUtil::DrawGrid();
    }
};

#endif // CGRIDRENDERABLE


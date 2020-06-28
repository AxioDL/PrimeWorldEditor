#ifndef CLINERENDERABLE
#define CLINERENDERABLE

#include <Common/CColor.h>
#include <Common/Math/CVector3f.h>
#include <Core/Render/CDrawUtil.h>
#include <Core/Render/CRenderer.h>
#include <Core/Render/IRenderable.h>

#include <array>

class CLineRenderable : public IRenderable
{
    std::array<CVector3f, 2> mPoints;
    CColor mColor;

public:
    CLineRenderable() : IRenderable() {}

    void SetPoints(const CVector3f& rkPointA, const CVector3f& rkPointB)
    {
        mPoints[0] = rkPointA;
        mPoints[1] = rkPointB;
    }

    void SetColor(const CColor& rkColor)
    {
        mColor = rkColor;
    }

    void AddToRenderer(CRenderer *pRenderer, const SViewInfo& /*rkViewInfo*/) override
    {
        pRenderer->AddMesh(this, -1, CAABox::Infinite(), false, ERenderCommand::DrawMesh);
    }

    void Draw(FRenderOptions, int, ERenderCommand, const SViewInfo&) override
    {
        CGraphics::sMVPBlock.ModelMatrix = CMatrix4f::skIdentity;
        CGraphics::UpdateMVPBlock();
        glDepthRange(0.f, 1.f);
        glLineWidth(1.f);
        CDrawUtil::DrawLine(mPoints[0], mPoints[1], mColor);
    }
};

#endif // CLINERENDERABLE


#include "CRadiusSphereExtra.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"

CRadiusSphereExtra::CRadiusSphereExtra(CScriptObject *pInstance, CSceneManager *pScene, CSceneNode *pParent)
    : CScriptExtra(pInstance, pScene, pParent)
    , mpRadius(nullptr)
{
    mObjectType = pInstance->ObjectTypeID();

    switch (mObjectType)
    {
    case 0x63: // Repulsor (MP1)
        mpRadius = (TFloatProperty*) pInstance->Properties()->PropertyByID(0x3);
        break;

    case 0x68: // RadialDamage (MP1)
        mpRadius = (TFloatProperty*) pInstance->Properties()->PropertyByID(0x4);
        break;

    case 0x5245504C: // "REPL" Repulsor (MP2/MP3)
    case 0x52414444: // "RADD" RadialDamage (MP2/MP3/DKCR)
        mpRadius = (TFloatProperty*)  pInstance->Properties()->PropertyByID(0x78C507EB);
        break;
    }
}

void CRadiusSphereExtra::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (!rkViewInfo.GameMode && mpRadius && mpParent->IsVisible() && mpParent->IsSelected())
    {
        CAABox BoundingBox = Bounds();

        if (rkViewInfo.ViewFrustum.BoxInFrustum(BoundingBox))
            pRenderer->AddOpaqueMesh(this, -1, BoundingBox, eDrawMesh);
    }
}

void CRadiusSphereExtra::Draw(FRenderOptions /*Options*/, int /*ComponentIndex*/, const SViewInfo& /*rkViewInfo*/)
{
    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    CDrawUtil::DrawWireSphere(mpInstance->Position(), mpRadius->Get(), Color());
}

CColor CRadiusSphereExtra::Color() const
{
    switch (mObjectType)
    {
    // Repulsor
    case 0x63:
    case 0x5245504C:
        return CColor::skGreen;

    // RadialDamage
    case 0x68:
    case 0x52414444:
        return CColor::skRed;

    default:
        return CColor::skWhite;
    }
}

CAABox CRadiusSphereExtra::Bounds() const
{
    CAABox Bounds = CAABox::skOne * 2.f * mpRadius->Get();
    Bounds += mpParent->AbsolutePosition();
    return Bounds;
}

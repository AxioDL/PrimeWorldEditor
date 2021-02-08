#include "CRadiusSphereExtra.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"

CRadiusSphereExtra::CRadiusSphereExtra(CScriptObject* pInstance, CScene* pScene, CScriptNode* pParent)
    : CScriptExtra(pInstance, pScene, pParent)
{
    mObjectType = pInstance->ObjectTypeID();
    CStructProperty* pProperties = pInstance->Template()->Properties();

    switch (mObjectType)
    {
    case 0x63: // Repulsor (MP1)
        mRadius = CFloatRef(pInstance->PropertyData(), pProperties->ChildByID(3));
        break;

    case 0x68: // RadialDamage (MP1)
        mRadius = CFloatRef(pInstance->PropertyData(), pProperties->ChildByID(0x4));
        break;

    case FOURCC('REPL'): // Repulsor (MP2/MP3)
    case FOURCC('RADD'): // RadialDamage (MP2/MP3/DKCR)
        mRadius = CFloatRef(pInstance->PropertyData(), pProperties->ChildByID(0x78C507EB));
        break;
    }
}

void CRadiusSphereExtra::AddToRenderer(CRenderer* pRenderer, const SViewInfo& rkViewInfo)
{
    if (!rkViewInfo.GameMode && (rkViewInfo.ShowFlags & EShowFlag::ObjectGeometry) && mRadius.IsValid() && mpParent->IsVisible() && mpParent->IsSelected())
    {
        CAABox BoundingBox = Bounds();

        if (rkViewInfo.ViewFrustum.BoxInFrustum(BoundingBox))
            pRenderer->AddMesh(this, -1, BoundingBox, false, ERenderCommand::DrawMesh);
    }
}

void CRadiusSphereExtra::Draw(FRenderOptions /*Options*/, int /*ComponentIndex*/, ERenderCommand /*Command*/, const SViewInfo& /*rkViewInfo*/)
{
    glBlendFunc(GL_ONE, GL_ZERO);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);

    CDrawUtil::DrawWireSphere(mpInstance->Position(), mRadius, Color());
}

CColor CRadiusSphereExtra::Color() const
{
    switch (mObjectType)
    {
    // Repulsor
    case 0x63:
    case FOURCC('REPL'):
        return CColor::Green();

    // RadialDamage
    case 0x68:
    case FOURCC('RADD'):
        return CColor::Red();

    default:
        return CColor::White();
    }
}

CAABox CRadiusSphereExtra::Bounds() const
{
    CAABox Bounds = CAABox::One() * 2.f * mRadius;
    Bounds += mpParent->AbsolutePosition();
    return Bounds;
}

#include "CLightNode.h"
#include <Core/CDrawUtil.h>
#include <Core/CGraphics.h>
#include <Core/CRenderer.h>

CLightNode::CLightNode(CSceneManager *pScene, CSceneNode *pParent, CLight *Light)
    : CSceneNode(pScene, pParent)
{
    mpLight = Light;
    mLocalAABox = CAABox::skOne;
    mPosition = Light->GetPosition();

    switch (Light->GetType())
    {
    case eLocalAmbient: SetName("Ambient Light");     break;
    case eDirectional:  SetName("Directional Light"); break;
    case eSpot:         SetName("Spot Light");        break;
    case eCustom:       SetName("Custom Light");      break;
    }
}

ENodeType CLightNode::NodeType()
{
    return eLightNode;
}

void CLightNode::AddToRenderer(CRenderer *pRenderer, const CFrustumPlanes& frustum)
{
    if (!frustum.BoxInFrustum(AABox())) return;

    pRenderer->AddOpaqueMesh(this, 0, CAABox(mPosition + 0.5f, mPosition - 0.5f), eDrawMesh);
}

void CLightNode::Draw(ERenderOptions /*Options*/)
{
    CDrawUtil::DrawLightBillboard(mpLight->GetType(), mpLight->GetColor(), mPosition, mScale.xy() * CVector2f(0.75f), TintColor());

    // Below commented-out code is for light radius visualization as a bounding box
    /*float r = mLight->GetRadius();
    CAABox AABB(Position + r, Position - r);
    pRenderer->DrawBoundingBox(mLight->GetColor(), AABB);*/
}

void CLightNode::DrawAsset(ERenderOptions /*Options*/, u32 /*asset*/)
{
}

SRayIntersection CLightNode::RayNodeIntersectTest(const CRay& Ray, u32 /*AssetID*/, ERenderOptions options)
{
    // Needs redo if I ever make these look like something other than boxes
    bool allowBackfaces = ((options & eEnableBackfaceCull) == 0);

    if (!allowBackfaces && (AABox().IsPointInBox(Ray.Origin())))
        return SRayIntersection(false, 0.f, nullptr, 0);

    std::pair<bool,float> BoxResult = AABox().IntersectsRay(Ray);

    if (BoxResult.first)
        return SRayIntersection(true, BoxResult.second, this, 0);

    else
        return SRayIntersection(false, 0.f, nullptr, 0);
}

CLight* CLightNode::Light()
{
    return mpLight;
}

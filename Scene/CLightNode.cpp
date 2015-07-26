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

void CLightNode::AddToRenderer(CRenderer *pRenderer)
{
    pRenderer->AddOpaqueMesh(this, 0, CAABox(mPosition + 0.5f, mPosition - 0.5f), eDrawMesh);

    if (IsSelected())
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawSelection);
}

void CLightNode::Draw(ERenderOptions)
{
    // Not using parameter 1 (ERenderOptions - Options)
    glBlendFunc(GL_ONE, GL_ZERO);
    glDepthMask(GL_TRUE);
    LoadModelMatrix();
    CGraphics::SetDefaultLighting();
    CGraphics::UpdateLightBlock();
    CGraphics::sVertexBlock.COLOR0_Amb = CVector4f(1.f);
    CGraphics::sVertexBlock.COLOR0_Mat = CVector4f(1.f);
    CGraphics::UpdateVertexBlock();
    CDrawUtil::DrawShadedCube(mpLight->GetColor());

    // Below commented-out code is for light radius visualization as a bounding box
    /*float r = mLight->GetRadius();
    CAABox AABB(Position + r, Position - r);
    pRenderer->DrawBoundingBox(mLight->GetColor(), AABB);*/
}

void CLightNode::DrawAsset(ERenderOptions, u32)
{
    // Not using parameter 1 (ERenderOptions - Options)
    // Not using parameter 2 (u32 - asset)
}

SRayIntersection CLightNode::RayNodeIntersectTest(const CRay &Ray, u32 AssetID)
{
    // Needs redo if I ever make these look like something other than boxes
    if (AABox().IsPointInBox(Ray.Origin()))
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

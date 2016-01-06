#include "CStaticNode.h"
#include "Core/Render/CGraphics.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include <Common/AnimUtil.h>
#include <Math/MathUtil.h>

CStaticNode::CStaticNode(CScene *pScene, CSceneNode *pParent, CStaticModel *pModel)
    : CSceneNode(pScene, pParent)
{
    mpModel = pModel;
    mLocalAABox = mpModel->AABox();
    mScale = CVector3f(1.f, 1.f, 1.f);
    SetName("Static Node");
}

ENodeType CStaticNode::NodeType()
{
    return eStaticNode;
}

void CStaticNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    if (!mpModel) return;
    if (mpModel->IsOccluder()) return;
    if (!ViewInfo.ViewFrustum.BoxInFrustum(AABox())) return;

    if (!mpModel->IsTransparent())
        pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawMesh);

    else
    {
        u32 NumSurfaces = mpModel->GetSurfaceCount();
        for (u32 iSurf = 0; iSurf < NumSurfaces; iSurf++)
        {
            CAABox TransformedBox = mpModel->GetSurfaceAABox(iSurf).Transformed(Transform());

            if (ViewInfo.ViewFrustum.BoxInFrustum(TransformedBox))
                pRenderer->AddTransparentMesh(this, iSurf, TransformedBox, eDrawMesh);
        }
    }

    if (mSelected && !ViewInfo.GameMode)
        pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawSelection);
}

void CStaticNode::Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo)
{
    if (!mpModel) return;

    bool IsLightingEnabled = CGraphics::sLightMode == CGraphics::eWorldLighting || ViewInfo.GameMode;
    CGraphics::sVertexBlock.COLOR0_Amb = (IsLightingEnabled ? CColor::skBlack : CColor::skWhite);
    float Mul = CGraphics::sWorldLightMultiplier;
    CGraphics::sPixelBlock.TevColor = CColor(Mul,Mul,Mul);
    CGraphics::sPixelBlock.TintColor = TintColor(ViewInfo);
    CGraphics::sNumLights = 0;
    CGraphics::UpdateLightBlock();
    LoadModelMatrix();

    if (ComponentIndex < 0)
        mpModel->Draw(Options);
    else
        mpModel->DrawSurface(Options, ComponentIndex);
}

void CStaticNode::DrawSelection()
{
    if (!mpModel) return;
    LoadModelMatrix();
    mpModel->DrawWireframe(eNoRenderOptions, WireframeColor());
}

void CStaticNode::RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& /*ViewInfo*/)
{
    if ((!mpModel) || (mpModel->IsOccluder()))
        return;

    const CRay& Ray = Tester.Ray();
    std::pair<bool,float> BoxResult = AABox().IntersectsRay(Ray);

    if (BoxResult.first)
    {
        for (u32 iSurf = 0; iSurf < mpModel->GetSurfaceCount(); iSurf++)
        {
            std::pair<bool,float> SurfResult = mpModel->GetSurfaceAABox(iSurf).Transformed(Transform()).IntersectsRay(Ray);

            if (SurfResult.first)
                Tester.AddNode(this, iSurf, SurfResult.second);
        }
    }
}

SRayIntersection CStaticNode::RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo)
{
    SRayIntersection out;
    out.pNode = this;
    out.ComponentIndex = AssetID;

    CRay TransformedRay = Ray.Transformed(Transform().Inverse());
    FRenderOptions options = ViewInfo.pRenderer->RenderOptions();
    std::pair<bool,float> Result = mpModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((options & eEnableBackfaceCull) == 0));

    if (Result.first)
    {
        out.Hit = true;

        CVector3f HitPoint = TransformedRay.PointOnRay(Result.second);
        CVector3f WorldHitPoint = Transform() * HitPoint;
        out.Distance = Math::Distance(Ray.Origin(), WorldHitPoint);
    }

    else
        out.Hit = false;

    return out;
}

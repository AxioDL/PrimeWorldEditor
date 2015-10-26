#include "CStaticNode.h"
#include <Core/CRenderer.h>
#include <Core/CGraphics.h>
#include <Common/AnimUtil.h>
#include <Common/Math.h>
#include <Core/CDrawUtil.h>

CStaticNode::CStaticNode(CSceneManager *pScene, CSceneNode *pParent, CStaticModel *pModel)
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

void CStaticNode::AddToRenderer(CRenderer *pRenderer, const CFrustumPlanes& frustum)
{
    if (!mpModel) return;
    if (mpModel->IsOccluder()) return;
    if (!frustum.BoxInFrustum(AABox())) return;

    if (!mpModel->IsTransparent())
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawMesh);

    else
    {
        u32 sm_count = mpModel->GetSurfaceCount();
        for (u32 s = 0; s < sm_count; s++)
        {
            if (frustum.BoxInFrustum(mpModel->GetSurfaceAABox(s).Transformed(Transform())))
                pRenderer->AddTransparentMesh(this, s, mpModel->GetSurfaceAABox(s).Transformed(Transform()), eDrawAsset);
        }
    }

    if (mSelected)
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawSelection);
}

void CStaticNode::Draw(ERenderOptions Options)
{
    if (!mpModel) return;

    CGraphics::sVertexBlock.COLOR0_Amb = CVector4f(0, 0, 0, 1);
    float Multiplier = CGraphics::sWorldLightMultiplier;
    CGraphics::sPixelBlock.TevColor = CVector4f(Multiplier,Multiplier,Multiplier,1);
    CGraphics::sNumLights = 0;
    CGraphics::UpdateLightBlock();
    LoadModelMatrix();

    mpModel->Draw(Options);
}

void CStaticNode::DrawAsset(ERenderOptions Options, u32 Asset)
{
    if (!mpModel) return;

    CGraphics::sVertexBlock.COLOR0_Amb = CVector4f(0,0,0,1);
    CGraphics::sPixelBlock.TevColor = CVector4f(1,1,1,1);
    CGraphics::sNumLights = 0;
    CGraphics::UpdateLightBlock();
    LoadModelMatrix();

    mpModel->DrawSurface(Options, Asset);
    //CDrawUtil::DrawWireCube(mpModel->GetSurfaceAABox(Asset), CColor::skWhite);
}

void CStaticNode::RayAABoxIntersectTest(CRayCollisionTester &Tester)
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

SRayIntersection CStaticNode::RayNodeIntersectTest(const CRay &Ray, u32 AssetID, ERenderOptions options)
{
    SRayIntersection out;
    out.pNode = this;
    out.AssetIndex = AssetID;

    CRay TransformedRay = Ray.Transformed(Transform().Inverse());
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

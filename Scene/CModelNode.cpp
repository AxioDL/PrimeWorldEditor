#include "CModelNode.h"
#include <Common/Math.h>
#include <Core/CRenderer.h>
#include <Core/CGraphics.h>

CModelNode::CModelNode(CSceneManager *pScene, CSceneNode *pParent, CModel *pModel) : CSceneNode(pScene, pParent)
{
    SetModel(pModel);
    mScale = CVector3f(1.f);
    mLightingEnabled = true;
    mForceAlphaOn = false;
}

ENodeType CModelNode::NodeType()
{
    return eModelNode;
}

void CModelNode::AddToRenderer(CRenderer *pRenderer)
{
    if (!mpModel) return;

    if (!mpModel->HasTransparency(mActiveMatSet))
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawMesh);

    else
    {
        u32 SurfaceCount = mpModel->GetSurfaceCount();

        for (u32 iSurf = 0; iSurf < SurfaceCount; iSurf++)
        {
            if (!mpModel->IsSurfaceTransparent(iSurf, mActiveMatSet))
                pRenderer->AddOpaqueMesh(this, iSurf, mpModel->GetSurfaceAABox(iSurf).Transformed(Transform()), eDrawAsset);
            else
                pRenderer->AddTransparentMesh(this, iSurf, mpModel->GetSurfaceAABox(iSurf).Transformed(Transform()), eDrawAsset);
        }
    }

    if (mSelected)
        pRenderer->AddOpaqueMesh(this, 0, AABox(), eDrawSelection);
}

void CModelNode::Draw(ERenderOptions Options)
{
    if (!mpModel) return;
    if (mForceAlphaOn) Options = (ERenderOptions) (Options & ~eNoAlpha);

    if (mLightingEnabled)
    {
        CGraphics::SetDefaultLighting();
        CGraphics::UpdateLightBlock();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor.ToVector4f();
    }
    else
    {
        CGraphics::sNumLights = 0;
        CGraphics::sVertexBlock.COLOR0_Amb = CColor::skBlack.ToVector4f();
    }

    CGraphics::sPixelBlock.TevColor = CVector4f(1,1,1,1);
    LoadModelMatrix();

    mpModel->Draw(Options, mActiveMatSet);
}

void CModelNode::DrawAsset(ERenderOptions Options, u32 Asset)
{
    if (!mpModel) return;
    if (mForceAlphaOn) Options = (ERenderOptions) (Options & ~eNoAlpha);

    if (mLightingEnabled)
    {
        CGraphics::SetDefaultLighting();
        CGraphics::UpdateLightBlock();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor.ToVector4f();
    }
    else
    {
        CGraphics::sNumLights = 0;
        CGraphics::sVertexBlock.COLOR0_Amb = CColor::skBlack.ToVector4f();
    }

    CGraphics::sPixelBlock.TevColor = CVector4f(1,1,1,1);
    LoadModelMatrix();

    mpModel->DrawSurface(Options, Asset, mActiveMatSet);
}

void CModelNode::RayAABoxIntersectTest(CRayCollisionTester &Tester)
{
    if (!mpModel) return;

    const CRay& Ray = Tester.Ray();
    std::pair<bool,float> BoxResult = AABox().IntersectsRay(Ray);

    if (BoxResult.first)
    {
        for (u32 iSurf = 0; iSurf < mpModel->GetSurfaceCount(); iSurf++)
        {
            std::pair<bool,float> SurfResult = mpModel->GetSurfaceAABox(iSurf).IntersectsRay(Ray);

            if (SurfResult.first)
                Tester.AddNode(this, iSurf, SurfResult.second);
        }
    }
}

SRayIntersection CModelNode::RayNodeIntersectTest(const CRay &Ray, u32 AssetID)
{
    SRayIntersection out;
    out.pNode = this;
    out.AssetIndex = AssetID;

    CRay TransformedRay = Ray.Transformed(Transform().Inverse());
    std::pair<bool,float> Result = mpModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, Transform());

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

void CModelNode::SetModel(CModel *pModel)
{
    mpModel = pModel;
    mModelToken = CToken(pModel);
    mActiveMatSet = 0;

    if (pModel)
    {
        SetName(pModel->Source());
        mLocalAABox = mpModel->AABox();
    }
}

void CModelNode::ForceAlphaEnabled(bool Enable)
{
    mForceAlphaOn = Enable;
}

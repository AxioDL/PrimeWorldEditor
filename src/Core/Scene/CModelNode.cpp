#include "CModelNode.h"
#include <Common/Math.h>
#include <Core/CDrawUtil.h>
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

void CModelNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& ViewInfo)
{
    if (!mpModel) return;
    if (!ViewInfo.ViewFrustum.BoxInFrustum(AABox())) return;
    if (ViewInfo.GameMode) return;

    if (!mpModel->HasTransparency(mActiveMatSet))
        pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawMesh);
    else
        AddSurfacesToRenderer(pRenderer, mpModel, mActiveMatSet, ViewInfo);

    if (mSelected)
        pRenderer->AddOpaqueMesh(this, -1, AABox(), eDrawSelection);
}

void CModelNode::Draw(ERenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo)
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
    CGraphics::sPixelBlock.TintColor = TintColor(ViewInfo).ToVector4f();
    LoadModelMatrix();

    if (ComponentIndex < 0)
        mpModel->Draw(Options, mActiveMatSet);
    else
        mpModel->DrawSurface(Options, ComponentIndex, mActiveMatSet);
}

void CModelNode::DrawSelection()
{
    if (!mpModel) return;
    LoadModelMatrix();
    mpModel->DrawWireframe(eNoRenderOptions, WireframeColor());
}

void CModelNode::RayAABoxIntersectTest(CRayCollisionTester& Tester, const SViewInfo& /*ViewInfo*/)
{
    if (!mpModel) return;

    const CRay& Ray = Tester.Ray();
    std::pair<bool,float> BoxResult = AABox().IntersectsRay(Ray);

    if (BoxResult.first)
        Tester.AddNodeModel(this, mpModel);
}

SRayIntersection CModelNode::RayNodeIntersectTest(const CRay &Ray, u32 AssetID, const SViewInfo& ViewInfo)
{
    SRayIntersection out;
    out.pNode = this;
    out.ComponentIndex = AssetID;

    CRay TransformedRay = Ray.Transformed(Transform().Inverse());
    ERenderOptions options = ViewInfo.pRenderer->RenderOptions();
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

void CModelNode::SetModel(CModel *pModel)
{
    mpModel = pModel;
    mActiveMatSet = 0;

    if (pModel)
    {
        SetName(pModel->Source());
        mLocalAABox = mpModel->AABox();
    }

    MarkTransformChanged();
}

void CModelNode::ForceAlphaEnabled(bool Enable)
{
    mForceAlphaOn = Enable;
}

#include "CModelNode.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/Render/CGraphics.h"
#include <Math/MathUtil.h>

CModelNode::CModelNode(CScene *pScene, CSceneNode *pParent, CModel *pModel) : CSceneNode(pScene, pParent)
{
    SetModel(pModel);
    mScale = CVector3f(1.f);
    mWorldModel = false;
    mForceAlphaOn = false;
    mEnableScanOverlay = false;
    mTintColor = CColor::skWhite;
}

ENodeType CModelNode::NodeType()
{
    return eModelNode;
}

void CModelNode::PostLoad()
{
    if (mpModel)
    {
        mpModel->BufferGL();
        mpModel->GenerateMaterialShaders();
    }
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

void CModelNode::Draw(FRenderOptions Options, int ComponentIndex, const SViewInfo& ViewInfo)
{
    if (!mpModel) return;
    if (mForceAlphaOn) Options = (FRenderOptions) (Options & ~eNoAlpha);

    if (!mWorldModel)
    {
        CGraphics::SetDefaultLighting();
        CGraphics::UpdateLightBlock();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor;
        CGraphics::sPixelBlock.LightmapMultiplier = 1.f;
        CGraphics::sPixelBlock.TevColor = CColor::skWhite;
    }
    else
    {
        bool IsLightingEnabled = CGraphics::sLightMode == CGraphics::eWorldLighting || ViewInfo.GameMode;

        if (IsLightingEnabled)
        {
            CGraphics::sNumLights = 0;
            CGraphics::sVertexBlock.COLOR0_Amb = CColor::skBlack;
            CGraphics::sPixelBlock.LightmapMultiplier = 1.f;
            CGraphics::UpdateLightBlock();
        }

        else
        {
            LoadLights(ViewInfo);
            if (CGraphics::sLightMode == CGraphics::eNoLighting)
                CGraphics::sVertexBlock.COLOR0_Amb = CColor::skWhite;
        }

        float Mul = CGraphics::sWorldLightMultiplier;
        CGraphics::sPixelBlock.TevColor = CColor(Mul,Mul,Mul);
    }

    CGraphics::sPixelBlock.TintColor = TintColor(ViewInfo);
    LoadModelMatrix();

    if (ComponentIndex < 0)
        mpModel->Draw(Options, mActiveMatSet);
    else
        mpModel->DrawSurface(Options, ComponentIndex, mActiveMatSet);

    if (mEnableScanOverlay)
    {
        CDrawUtil::UseColorShader(mScanOverlayColor);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ZERO);
        Options |= eNoMaterialSetup;

        if (ComponentIndex < 0)
            mpModel->Draw(Options, 0);
        else
            mpModel->DrawSurface(Options, ComponentIndex, mActiveMatSet);
    }
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

CColor CModelNode::TintColor(const SViewInfo& /*rkViewInfo*/) const
{
    return mTintColor;
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

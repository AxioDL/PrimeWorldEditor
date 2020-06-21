#include "CModelNode.h"
#include "Core/Render/CDrawUtil.h"
#include "Core/Render/CRenderer.h"
#include "Core/Render/CGraphics.h"
#include <Common/Math/MathUtil.h>

CModelNode::CModelNode(CScene *pScene, uint32 NodeID, CSceneNode *pParent, CModel *pModel)
    : CSceneNode(pScene, NodeID, pParent)
{
    mScale = CVector3f::One();
    SetModel(pModel);
}

ENodeType CModelNode::NodeType()
{
    return ENodeType::Model;
}

void CModelNode::PostLoad()
{
    if (mpModel)
    {
        mpModel->BufferGL();
        mpModel->GenerateMaterialShaders();
    }
}

void CModelNode::AddToRenderer(CRenderer *pRenderer, const SViewInfo& rkViewInfo)
{
    if (!mpModel)
        return;
 
    if (!rkViewInfo.ViewFrustum.BoxInFrustum(AABox()))
        return;

    if (rkViewInfo.GameMode)
        return;

    // Transparent world models should have each surface processed separately
    if (mWorldModel && mpModel->HasTransparency(mActiveMatSet))
    {
        pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawOpaqueParts);

        for (uint32 iSurf = 0; iSurf < mpModel->GetSurfaceCount(); iSurf++)
        {
            if (mpModel->IsSurfaceTransparent(iSurf, mActiveMatSet))
                pRenderer->AddMesh(this, iSurf, mpModel->GetSurfaceAABox(iSurf).Transformed(Transform()), true, ERenderCommand::DrawTransparentParts);
        }
    }
    else // Other models should just draw all transparent surfaces sequentially
    {
        AddModelToRenderer(pRenderer, mpModel, mActiveMatSet);
    }

    if (mSelected)
        pRenderer->AddMesh(this, -1, AABox(), false, ERenderCommand::DrawSelection);
}

void CModelNode::Draw(FRenderOptions Options, int ComponentIndex, ERenderCommand Command, const SViewInfo& rkViewInfo)
{
    if (!mpModel)
        return;

    if (mForceAlphaOn)
        Options = static_cast<FRenderOptions>(Options & ~ERenderOption::NoAlpha);

    if (!mWorldModel)
    {
        CGraphics::SetDefaultLighting();
        CGraphics::UpdateLightBlock();
        CGraphics::sVertexBlock.COLOR0_Amb = CGraphics::skDefaultAmbientColor;
        CGraphics::sVertexBlock.COLOR0_Mat = CColor::TransparentWhite();
        CGraphics::sPixelBlock.LightmapMultiplier = 1.f;
        CGraphics::sPixelBlock.SetAllTevColors(CColor::White());
    }
    else
    {
        const bool IsLightingEnabled = CGraphics::sLightMode == CGraphics::ELightingMode::World || rkViewInfo.GameMode;

        if (IsLightingEnabled)
        {
            CGraphics::sNumLights = 0;
            CGraphics::sVertexBlock.COLOR0_Amb = CColor::TransparentBlack();
            CGraphics::sPixelBlock.LightmapMultiplier = 1.f;
            CGraphics::UpdateLightBlock();
        }
        else
        {
            LoadLights(rkViewInfo);
            if (CGraphics::sLightMode == CGraphics::ELightingMode::None)
                CGraphics::sVertexBlock.COLOR0_Amb = CColor::TransparentWhite();
        }

        CGraphics::sVertexBlock.COLOR0_Mat = CColor::TransparentWhite();

        const float Mul = CGraphics::sWorldLightMultiplier;
        CGraphics::sPixelBlock.SetAllTevColors(CColor(Mul,Mul,Mul));
    }

    CGraphics::sPixelBlock.TintColor = TintColor(rkViewInfo);
    LoadModelMatrix();

    if (mpModel->IsSkinned())
        CGraphics::LoadIdentityBoneTransforms();

    if (ComponentIndex == -1)
        DrawModelParts(mpModel, Options, mActiveMatSet, Command);
    else
        mpModel->DrawSurface(Options, ComponentIndex, mActiveMatSet);

    if (mEnableScanOverlay)
    {
        CDrawUtil::UseColorShader(mScanOverlayColor);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ZERO);
        Options |= ERenderOption::NoMaterialSetup;
        DrawModelParts(mpModel, Options, 0, Command);
    }
}

void CModelNode::DrawSelection()
{
    if (!mpModel)
        return;

    LoadModelMatrix();
    mpModel->DrawWireframe(ERenderOption::None, WireframeColor());
}

void CModelNode::RayAABoxIntersectTest(CRayCollisionTester& rTester, const SViewInfo& /*rkViewInfo*/)
{
    if (!mpModel)
        return;

    const CRay& rkRay = rTester.Ray();
    const std::pair<bool, float> BoxResult = AABox().IntersectsRay(rkRay);

    if (BoxResult.first)
        rTester.AddNodeModel(this, mpModel);
}

SRayIntersection CModelNode::RayNodeIntersectTest(const CRay& rkRay, uint32 AssetID, const SViewInfo& rkViewInfo)
{
    SRayIntersection Out;
    Out.pNode = this;
    Out.ComponentIndex = AssetID;

    const CRay TransformedRay = rkRay.Transformed(Transform().Inverse());
    const FRenderOptions Options = rkViewInfo.pRenderer->RenderOptions();
    const auto [intersects, distance] = mpModel->GetSurface(AssetID)->IntersectsRay(TransformedRay, ((Options & ERenderOption::EnableBackfaceCull) == 0));

    if (intersects)
    {
        Out.Hit = true;

        const CVector3f HitPoint = TransformedRay.PointOnRay(distance);
        const CVector3f WorldHitPoint = Transform() * HitPoint;
        Out.Distance = Math::Distance(rkRay.Origin(), WorldHitPoint);
    }
    else
    {
        Out.Hit = false;
    }

    return Out;
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
